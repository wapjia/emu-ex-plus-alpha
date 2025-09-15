/*  This file is part of Imagine.

	Imagine is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Imagine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Imagine.  If not, see <http://www.gnu.org/licenses/> */

#include <unistd.h>
#include <cerrno>
#include <imagine/base/Screen.hh>
#include <imagine/base/ApplicationContext.hh>
#include <imagine/base/Application.hh>
#include <imagine/time/Time.hh>
#include <imagine/util/algorithm.h>
#include <imagine/logger/logger.h>
#include "android.hh"
#include <imagine/base/SimpleFrameTimer.hh>

namespace IG
{

constexpr SystemLogger log{"Screen"};
static JNI::InstMethod<void(jboolean)> jSetListener{};
static JNI::InstMethod<void(jlong)> jEnumDisplays{};

void AndroidApplication::initScreens(JNIEnv *env, jobject baseActivity, jclass baseActivityClass, int32_t androidSDK, ANativeActivity *nActivity)
{
	assert(!jEnumDisplays);
	if(androidSDK >= 17)
	{
		JNI::InstMethod<jobject(jlong)> jDisplayListenerHelper{env, baseActivityClass, "displayListenerHelper", "(J)Lcom/imagine/DisplayListenerHelper;"};
		displayListenerHelper = {env, jDisplayListenerHelper(env, baseActivity, (jlong)nActivity)};
		auto displayListenerHelperCls = env->GetObjectClass(displayListenerHelper);
		JNINativeMethod method[]
		{
			{
				"displayAdd", "(JILandroid/view/Display;FJLandroid/util/DisplayMetrics;)V",
				(void*)
				+[](JNIEnv* env, jobject, jlong nActivityAddr, jint id, jobject disp, jfloat refreshRate, jlong presentationDeadline, jobject metrics)
				{
					ApplicationContext ctx{(ANativeActivity*)nActivityAddr};
					auto &app = ctx.application();
					if(app.findScreen(id))
					{
						log.info("screen id:{} already in device list", id);
						return;
					}
					app.addScreen(ctx, std::make_unique<Screen>(ctx,
						Screen::InitParams{env, disp, metrics, id, refreshRate, Nanoseconds{presentationDeadline}, Rotation::UP}), true);
				}
			},
			{
				"displayChange", "(JIFJ)V",
				(void*)
				+[](JNIEnv*, jobject, jlong nActivityAddr, jint id, jfloat refreshRate, jlong presentationDeadline)
				{
					ApplicationContext ctx{(ANativeActivity*)nActivityAddr};
					auto &app = ctx.application();
					auto screen = app.findScreen(id);
					if(!screen)
					{
						log.warn("screen id:{} changed but isn't in device list", id);
						return;
					}
					if(screen->updateFrameRate(refreshRate, Nanoseconds{presentationDeadline}))
						app.dispatchOnScreenChange(ctx, *screen, ScreenChange::frameRate);
				}
			},
			{
				"displayRemove", "(JI)V",
				(void*)
				+[](JNIEnv*, jobject, jlong nActivityAddr, jint id)
				{
					ApplicationContext ctx{(ANativeActivity*)nActivityAddr};
					auto &app = ctx.application();
					log.info("screen id:{} removed", id);
					app.removeScreen(ctx, id, true);
				}
			}
		};
		env->RegisterNatives(displayListenerHelperCls, method, std::size(method));
		jSetListener = {env, displayListenerHelperCls, "setListener", "(Z)V"};
		jSetListener(env, displayListenerHelper, true);
		addOnExit([env, &displayListenerHelper = displayListenerHelper](ApplicationContext ctx, bool backgrounded)
		{
			ctx.application().removeSecondaryScreens();
			log.info("unregistering display listener");
			jSetListener(env, displayListenerHelper, false);
			if(backgrounded)
			{
				ctx.addOnResume([env, &displayListenerHelper](ApplicationContext ctx, bool)
				{
					log.info("registering display listener");
					jSetListener(env, displayListenerHelper, true);
					jEnumDisplays(env, ctx.baseActivityObject(), (jlong)ctx.aNativeActivityPtr());
					return false;
				}, SCREEN_ON_RESUME_PRIORITY);
			}
			return true;
		}, SCREEN_ON_EXIT_PRIORITY);
	}
	jEnumDisplays = {env, baseActivityClass, "enumDisplays", "(J)V"};
	jEnumDisplays(env, baseActivity, (jlong)nActivity);
}

AndroidScreen::AndroidScreen(ApplicationContext ctx, InitParams params)
{
	auto [env, aDisplay, metrics, id, refreshRate, presentationDeadline, rotation] = params;
	assert(aDisplay);
	assert(metrics);
	this->aDisplay = {env, aDisplay};
	bool isStraightRotation = true;
	if(id == 0)
	{
		id_ = 0;
		log.info("init main display with starting rotation:{}", (int)rotation);
		ctx.application().setCurrentRotation(ctx, rotation);
		isStraightRotation = !isSideways(rotation);
	}
	else
	{
		id_ = id;
		log.info("init display with id:{}", id_);
	}
	updateFrameRate(refreshRate, presentationDeadline);
	if(ctx.androidSDK() <= 10)
	{
		// corrections for devices known to report wrong refresh rates
		auto buildDevice = ctx.androidBuildDevice();
		if(Config::MACHINE_IS_GENERIC_ARMV7 && buildDevice == "R800at")
		{
			frameRate_ = 61.5;
		}
		else if(Config::MACHINE_IS_GENERIC_ARMV7 && buildDevice == "sholes")
		{
			frameRate_ = 60;
		}
	}
	updateSupportedFrameRates(ctx, env);

	// DisplayMetrics
	jclass jDisplayMetricsCls = env->GetObjectClass(metrics);
	auto jDensity = env->GetFieldID(jDisplayMetricsCls, "density", "F");
	auto jScaledDensity = env->GetFieldID(jDisplayMetricsCls, "scaledDensity", "F");
	auto jWidthPixels = env->GetFieldID(jDisplayMetricsCls, "widthPixels", "I");
	auto jHeightPixels = env->GetFieldID(jDisplayMetricsCls, "heightPixels", "I");
	auto widthPixels = env->GetIntField(metrics, jWidthPixels);
	auto heightPixels = env->GetIntField(metrics, jHeightPixels);
	densityDPI_ = 160.*env->GetFloatField(metrics, jDensity);
	assert(densityDPI_);
	scaledDensityDPI_ = 160.*env->GetFloatField(metrics, jScaledDensity);
	assert(scaledDensityDPI_);
	log.info("screen with size:{}x{}, density DPI:{}, scaled density DPI:{}",
		widthPixels, heightPixels, densityDPI_, scaledDensityDPI_);
	if(Config::DEBUG_BUILD)
	{
		auto jXDPI = env->GetFieldID(jDisplayMetricsCls, "xdpi", "F");
		auto jYDPI = env->GetFieldID(jDisplayMetricsCls, "ydpi", "F");
		auto metricsXDPI = env->GetFloatField(metrics, jXDPI);
		auto metricsYDPI = env->GetFloatField(metrics, jYDPI);
		// DPI values are un-rotated from DisplayMetrics
		if(!isStraightRotation)
			std::swap(metricsXDPI, metricsYDPI);
		auto jDensityDPI = env->GetFieldID(jDisplayMetricsCls, "densityDpi", "I");
		log.info("DPI:{}x{}, densityDPI:{}, refresh rate:{}Hz",
			metricsXDPI, metricsYDPI, env->GetIntField(metrics, jDensityDPI),
			frameRate_.hz());
	}
	if(!isStraightRotation)
		std::swap(widthPixels, heightPixels);
	width_ = widthPixels;
	height_ = heightPixels;
}

bool AndroidScreen::updateFrameRate(float rate, Nanoseconds presentationDeadline)
{
	if(frameRate_ && rate == frameRate_.hz())
		return false;
	log.info("refresh rate updated to:{} deadline:{} on screen:{}", rate, presentationDeadline, id());
	if(rate < 1.f || rate > 700.f) // sanity check in case device has a junk value
	{
		log.warn("ignoring unusual refresh rate:{}", rate);
		rate = 60;
		reliableFrameRate = false;
	}
	frameRate_ = rate;
	targetFrameDuration_ = presentationDeadline >= frameRate_.duration() ? Milliseconds{4} : frameRate_.duration() - presentationDeadline;
	return true;
}

void AndroidScreen::updateSupportedFrameRates(ApplicationContext ctx, JNIEnv *env)
{
	if(ctx.androidSDK() < 21)
	{
		return;
	}
	doIfUsed(supportedFrameRates_, [&](auto& supportedRates)
	{
		JNI::InstMethod<jobject()> jGetSupportedRefreshRates{env, (jobject)aDisplay, "getSupportedRefreshRates", "()[F"};
		auto jRates = (jfloatArray)jGetSupportedRefreshRates(env, aDisplay);
		std::span rates{env->GetFloatArrayElements(jRates, 0), size_t(env->GetArrayLength(jRates))};
		log.debug("screen {} supports {} rate(s):{}", id_, rates.size(), rates);
		reliableFrameRate = rates.size() > 1; // assume rate is reliable on devices with multiple rates
		supportedRates.insert_range(rates);
		env->ReleaseFloatArrayElements(jRates, rates.data(), 0);
	});
}

int Screen::width() const { return width_; }
int Screen::height() const { return height_; }
FrameRate Screen::frameRate() const { return frameRate_; }
SteadyClockDuration Screen::targetFrameDuration() const { return targetFrameDuration_; }
bool Screen::frameRateIsReliable() const { return reliableFrameRate; }

bool Screen::supportsTimestamps() const
{
	return appContext().androidSDK() >= 16;
}

void Screen::setFrameRate(FrameRate rate)
{
	frameTimer.setFrameRate(rate ?: frameRate());
}

std::span<const FrameRate> Screen::supportedFrameRates() const
{
	return doIfUsedOr(supportedFrameRates_,
		[&](auto& rates){ return std::span{std::addressof(*rates.begin()), rates.size()}; },
		[&](){ return std::span{&frameRate_, 1}; });
}

}
