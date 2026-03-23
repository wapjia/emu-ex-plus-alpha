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

#include <imagine/base/android/RootCpufreqParamSetter.hh>
#include <imagine/io/PosixIO.hh>
#include <imagine/util/utility.hh>
#include <imagine/logger/SystemLogger.hh>
#include <stdio.h>

#define TIMER_RATE_PATH "/sys/devices/system/cpu/cpufreq/interactive/timer_rate"
#define UP_THRESHOLD_PATH "/sys/devices/system/cpu/cpufreq/ondemand/up_threshold"
#define SAMPLING_RATE_PATH "/sys/devices/system/cpu/cpufreq/ondemand/sampling_rate"
#define SAMPLING_RATE_MiN_PATH "/sys/devices/system/cpu/cpufreq/ondemand/sampling_rate_min"

namespace IG
{

static SystemLogger log{"CpufreqParam"};

static int readIntFileValue(const char *path)
{
	try
	{
		PosixIO f{path};
		std::array<char, 32> buff{};
		f.read(buff.data(), buff.size() - 1, 0);
		int val = -1;
		std::sscanf(buff.data(), "%d", &val);
		return val;
	}
	catch(...)
	{
		return -1;
	}
}

RootCpufreqParamSetter::RootCpufreqParamSetter()
{
	origTimerRate = readIntFileValue(TIMER_RATE_PATH);
	if(origTimerRate <= 0)
	{
		origUpThreshold = readIntFileValue(UP_THRESHOLD_PATH);
		log.info("default up_threshold:{}", origUpThreshold);
		origSamplingRate = readIntFileValue(SAMPLING_RATE_PATH);
		log.info("default sampling_rate:{}", origSamplingRate);
	}
	else
	{
		log.info("default timer_rate:{}", origTimerRate);
	}

	if(origTimerRate <= 0 && origUpThreshold <= 0 && origSamplingRate <= 0)
	{
		log.error("couldn't read any cpufreq parameters");
		return;
	}

	rootShell = popen("su", "w");
	if(!rootShell)
	{
		log.error("error running root shell");
		return;
	}
	log.info("opened root shell");
}

RootCpufreqParamSetter::~RootCpufreqParamSetter()
{
	if(rootShell)
	{
		pclose(rootShell);
		log.info("closed root shell");
	}
}

void RootCpufreqParamSetter::setLowLatency()
{
	if(origTimerRate > 0)
	{
		// interactive
		log.info("setting low-latency interactive governor values");
		std::fprintf(rootShell, "echo -n 6000 > " TIMER_RATE_PATH "\n");
	}
	else
	{
		// ondemand
		log.info("setting low-latency ondemand governor values");
		if(origUpThreshold > 0)
			std::fprintf(rootShell, "echo -n 40 > " UP_THRESHOLD_PATH "\n");
		if(origSamplingRate > 0)
			std::fprintf(rootShell, "echo -n `cat " SAMPLING_RATE_MiN_PATH "` > " SAMPLING_RATE_PATH "\n");
	}
	std::fflush(rootShell);
}

void RootCpufreqParamSetter::setDefaults()
{
	if(origTimerRate > 0)
	{
		// interactive
		log.info("setting default interactive governor values");
		std::fprintf(rootShell, "echo -n %d > " TIMER_RATE_PATH "\n", origTimerRate);
	}
	else
	{
		// ondemand
		log.info("setting default ondemand governor values");
		if(origUpThreshold > 0)
			std::fprintf(rootShell, "echo -n %d > " UP_THRESHOLD_PATH "\n", origUpThreshold);
		if(origSamplingRate > 0)
			std::fprintf(rootShell, "echo -n %d > " SAMPLING_RATE_PATH "\n", origSamplingRate);
	}
	std::fflush(rootShell);
}

}
