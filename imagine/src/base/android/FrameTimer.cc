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

#include <imagine/base/android/Choreographer.hh>
#include <imagine/base/android/AndroidApplication.hh>
#include <imagine/base/sharedLibrary.hh>
#include <imagine/util/utility.hh>
#include <imagine/logger/SystemLogger.hh>
#include <android/choreographer.h>
#include <android/native_activity.h>
#include <unistd.h>
#include <jni.h>

namespace IG
{

constexpr SystemLogger log{"Choreographer"};

void AndroidApplication::emplaceFrameTimer(FrameTimer &t, Screen &screen, bool useVariableTime)
{
	if(useVariableTime)
	{
		t.emplace<SimpleFrameTimer>(screen);
	}
	else
	{
		return visit(overloaded
		{
			[&](JavaChoreographer &c)
			{
				t.emplace<JavaChoreographerFrameTimer>(c);
			},
			[&](NativeChoreographer &c)
			{
				if(c)
					t.emplace<NativeChoreographerFrameTimer>(c);
				else // no choreographer
					t.emplace<SimpleFrameTimer>(screen);
			},
		}, choreographer);
	}
}

void AndroidApplication::initChoreographer(JNIEnv *env, jobject baseActivity, jclass baseActivityClass, int32_t androidSDK)
{
	if(androidSDK < 16)
	{
		return;
	}
	if(sizeof(long) < sizeof(int64_t) || androidSDK < 24)
	{
		// Always use on 32-bit systems due to NDK API issue
		// that prevents 64-bit timestamp resolution
		choreographer.emplace<JavaChoreographer>(*this, env, baseActivity, baseActivityClass);
	}
	else
	{
		choreographer.emplace<NativeChoreographer>(*this);
	}
}

static void updatePostedScreens(auto& choreographer, SteadyClockTimePoint time, AndroidApplication& app)
{
	bool didUpdate{};
	for(auto &s : app.screens())
	{
		if(s->isPosted())
		{
			didUpdate |= s->frameUpdate(time);
		}
	}
	if(!didUpdate)
	{
		//log.debug("stopping screen updates");
		choreographer.cancel();
	}
}

JavaChoreographer::JavaChoreographer(AndroidApplication &app, JNIEnv *env, jobject baseActivity, jclass baseActivityClass):
	appPtr{&app}
{
	jniEnv = env;
	JNI::InstMethod<jobject(jlong)> jChoreographerHelper{env, baseActivityClass, "choreographerHelper", "(J)Lcom/imagine/ChoreographerHelper;"};
	frameHelper = {env, jChoreographerHelper(env, baseActivity, (jlong)this)};
	auto choreographerHelperCls = env->GetObjectClass(frameHelper);
	jPostFrame = {env, choreographerHelperCls, "postFrame", "()V"};
	jSetInstance = {env, choreographerHelperCls, "setInstance", "(Z)V"};
	JNINativeMethod method[]
	{
		{
			"onFrame", "(JJ)V",
			(void*)
			+[](JNIEnv*, jobject, jlong userData, jlong frameTimeNanos)
			{
				auto &inst = *((JavaChoreographer*)userData);
				if(!inst.requested) [[unlikely]]
					return;
				inst.requested = false;
				updatePostedScreens(inst, SteadyClockTimePoint{Nanoseconds{frameTimeNanos}}, *inst.appPtr);
			}
		}
	};
	env->RegisterNatives(choreographerHelperCls, method, std::size(method));
	log.info("created java choreographer");
}

void JavaChoreographer::scheduleVSync()
{
	assume(frameHelper);
	if(requested)
		return;
	requested = true;
	jPostFrame(jniEnv, frameHelper);
}

void JavaChoreographer::setEventsOnThisThread(ApplicationContext ctx)
{
	jniEnv = ctx.thisThreadJniEnv();
	jSetInstance(jniEnv, frameHelper, true);
}

void JavaChoreographer::removeEvents(ApplicationContext ctx)
{
	cancel();
	jniEnv = ctx.thisThreadJniEnv();
	jSetInstance(jniEnv, frameHelper, false);
}

NativeChoreographer::NativeChoreographer(AndroidApplication &app):
	appPtr{&app}
{
	loadSymbol(getInstance, {}, "AChoreographer_getInstance");
	assume(getInstance);
	loadSymbol(postFrameCallback, {}, "AChoreographer_postFrameCallback");
	assume(postFrameCallback);
	choreographer = getInstance();
	assume(choreographer);
	log.info("created native choreographer");
}

void NativeChoreographer::scheduleVSync()
{
	if(requested)
		return;
	requested = true;
	postFrameCallback(choreographer, [](long frameTimeNanos, void* userData)
	{
		auto &inst = *((NativeChoreographer*)userData);
		if(!inst.requested || inst.choreographer != inst.getInstance()) [[unlikely]]
			return;
		inst.requested = false;
		updatePostedScreens(inst, SteadyClockTimePoint{Nanoseconds{frameTimeNanos}}, *inst.appPtr);
	}, this);
}

void NativeChoreographer::setEventsOnThisThread(ApplicationContext)
{
	choreographer = getInstance();
}

void NativeChoreographer::removeEvents(ApplicationContext)
{
	cancel();
}

}
