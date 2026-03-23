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

#include <imagine/base/PerformanceHintManager.hh>
#include <imagine/base/ApplicationContext.hh>
#include <imagine/base/sharedLibrary.hh>
#include <imagine/util/utility.hh>
#include <imagine/logger/SystemLogger.hh>
#include <android/performance_hint.h>

namespace IG
{

[[maybe_unused]] static SystemLogger log{"PerfHint"};

static APerformanceHintSession* (*APerformanceHint_createSession)(APerformanceHintManager*,
	const int32_t* threadIds, size_t size, int64_t initialTargetWorkDurationNanos);
//static int64_t (*APerformanceHint_getPreferredUpdateRateNanos)(APerformanceHintManager*); // unused
static int (*APerformanceHint_updateTargetWorkDuration)(APerformanceHintSession*, int64_t targetDurationNanos);
static int (*APerformanceHint_reportActualWorkDuration)(APerformanceHintSession*, int64_t actualDurationNanos);
static void (*APerformanceHint_closeSession)(APerformanceHintSession*);

static void loadPerformanceHintFunc()
{
	loadSymbol(APerformanceHint_createSession, {}, "APerformanceHint_createSession");
	loadSymbol(APerformanceHint_updateTargetWorkDuration, {}, "APerformanceHint_updateTargetWorkDuration");
	loadSymbol(APerformanceHint_reportActualWorkDuration, {}, "APerformanceHint_reportActualWorkDuration");
	loadSymbol(APerformanceHint_closeSession, {}, "APerformanceHint_closeSession");
}

PerformanceHintManager ApplicationContext::performanceHintManager()
{
	if(androidSDK() < __ANDROID_API_T__)
	{
		return {};
	}
	APerformanceHintManager* (*APerformanceHint_getManager)(){};
	loadSymbol(APerformanceHint_getManager, {}, "APerformanceHint_getManager");
	if(!APerformanceHint_createSession)
		loadPerformanceHintFunc();
	return APerformanceHint_getManager();
}

PerformanceHintSession PerformanceHintManager::session(std::span<const ThreadId> threadIds, Nanoseconds initialTargetWorkDuration)
{
	if(!mgrPtr)
		return {};
	return APerformanceHint_createSession(mgrPtr, threadIds.data(), threadIds.size(), initialTargetWorkDuration.count());
}

PerformanceHintManager::operator bool() const { return mgrPtr; }

void PerformanceHintSession::updateTargetWorkDuration(Nanoseconds targetTime)
{
	if(!sessionPtr)
		return;
	if(APerformanceHint_updateTargetWorkDuration(sessionPtr.get(), targetTime.count()))
		log.error("error in APerformanceHint_updateTargetWorkDuration({}, {})", (void*)sessionPtr.get(), targetTime.count());
}

void PerformanceHintSession::reportActualWorkDuration(Nanoseconds actualTime)
{
	if(!sessionPtr)
		return;
	if(APerformanceHint_reportActualWorkDuration(sessionPtr.get(), actualTime.count()))
		log.error("error in APerformanceHint_reportActualWorkDuration({}, {})", (void*)sessionPtr.get(), actualTime.count());
}

PerformanceHintSession::operator bool() const { return bool(sessionPtr); }

void AndroidPerformanceHintSession::closeSession(APerformanceHintSession* session)
{
	APerformanceHint_closeSession(session);
}

}
