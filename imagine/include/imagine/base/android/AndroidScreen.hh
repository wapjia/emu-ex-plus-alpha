#pragma once

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

#include <imagine/config/defs.hh>
#include <imagine/time/Time.hh>
#include <imagine/base/baseDefs.hh>
#include <imagine/base/ApplicationContext.hh>
#include <imagine/base/SimpleFrameTimer.hh>
#include <imagine/base/android/Choreographer.hh>
#include <imagine/base/FrameTimerInterface.hh>
#include <imagine/util/jni.hh>
#include <imagine/util/used.hh>
#ifndef IG_USE_MODULE_STD
#include <variant>
#include <flat_set>
#endif

namespace IG
{
class ApplicationContext;
}

namespace IG
{

using FrameTimerVariant = std::variant<NativeChoreographerFrameTimer, JavaChoreographerFrameTimer, SimpleFrameTimer>;

class FrameTimer : public FrameTimerInterface<FrameTimerVariant>
{
public:
	using FrameTimerInterface::FrameTimerInterface;
};

using ScreenId = int;

class AndroidScreen
{
public:
	struct InitParams
	{
		JNIEnv* env;
		jobject aDisplay;
		jobject metrics;
		int id;
		float refreshRate;
		Nanoseconds presentationDeadline;
		Rotation rotation;
	};

	AndroidScreen(ApplicationContext, InitParams);
	float densityDPI() const { return densityDPI_; }
	float scaledDensityDPI() const { return scaledDensityDPI_; }
	jobject displayObject() const { return aDisplay; }
	int id() const { return id_;  }
	bool operator==(AndroidScreen const &rhs) const { return id_ == rhs.id_; }
	bool operator==(ScreenId id) const { return id_ == id; }
	explicit operator bool() const { return aDisplay; }
	bool updateFrameRate(float rate, Nanoseconds presentationDeadline);
	void updateSupportedFrameRates(ApplicationContext, JNIEnv*);

protected:
	JNI::UniqueGlobalRef aDisplay;
	FrameRate frameRate_{};
	SteadyClockDuration targetFrameDuration_{};
	ConditionalMember<Config::multipleScreenFrameRates, std::flat_set<FrameRate>> supportedFrameRates_;
	float densityDPI_{};
	float scaledDensityDPI_{};
	int width_{}, height_{};
	int id_{};
	ConditionalMember<Config::multipleScreenFrameRates, bool> reliableFrameRate{};
};

using ScreenImpl = AndroidScreen;

}
