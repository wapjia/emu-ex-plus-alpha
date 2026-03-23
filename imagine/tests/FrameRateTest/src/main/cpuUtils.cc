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

module;
#include <unistd.h>

module cpuUtils;
import imagine;

namespace cpuUtils
{

struct CPUTime
{
	unsigned long long user = 0;
	unsigned long long nice = 0;
	unsigned long long systemAll = 0;
	unsigned long long idle = 0;
	unsigned long long steal = 0;
	unsigned long long virt = 0;
	unsigned long long total = 0;
};

constexpr IG::SystemLogger log{"cpuStat"};
static CPUTime cpuTime;
static IG::FileIO cpuFreqFile{};
static std::FILE *procStatFile{};

CpuFreqString updateCPUFreq()
{
	if(!cpuFreqFile)
		return {};
	CpuFreqString buff{};
	cpuFreqFile.read(buff.data(), buff.size() - 1, 0);
	// remove any whitespace
	CpuFreqString str{};
	std::sscanf(buff.data(), "%s", str.data());
	//logMsg("CPU freq:%s", str);
	return str;
}

void initCPUFreqStatus()
{
	if(!Config::envIsLinux && !Config::envIsAndroid)
		return; // ignore cpufreq monitoring on non-linux systems
	if(cpuFreqFile)
		return; // already open
	const char *cpuFreqPath = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq";
	try
	{
		cpuFreqFile = {cpuFreqPath};
	}
	catch(...)
	{
		log.warn("can't open:{}", cpuFreqPath);
	}
}

void deinitCPUFreqStatus() {}

std::string updateCPULoad()
{
	if(!procStatFile)
		return {};
	auto file = std::freopen("/proc/stat", "r", procStatFile);
	if(!file)
	{
		procStatFile = nullptr;
		return "Error reading stats";
	}
	unsigned long long user = 0,
		nice = 0, system = 0, idle = 0,
		iowait = 0, irq = 0, softirq = 0,
		steal = 0, guest = 0, guestnice = 0;
	if(std::fscanf(procStatFile, "cpu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
		&user, &nice, &system, &idle,
		&iowait, &irq, &softirq,
		&steal, &guest, &guestnice) != 10)
	{
		return "Error reading stats";
	}
	/*logMsg("CPU times:%llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
		user, nice, system, idle,
		iowait, irq, softirq,
		steal, guest, guestnice);*/
	CPUTime newTime;
	newTime.user = user - guest;
	newTime.nice = nice - guestnice;
	auto idleAllTime = idle + iowait;
	newTime.systemAll = system + irq + softirq;
	newTime.virt = guest + guestnice;
	newTime.steal = steal;
	newTime.total = newTime.user + newTime.nice + newTime.systemAll
		+ idleAllTime + newTime.steal + newTime.virt;
	std::string useStr{"Calculating..."};
	if(cpuTime.user)
	{
		double userDelta = newTime.user - cpuTime.user;
		double niceDelta = newTime.nice - cpuTime.nice;
		double systemAllDelta = newTime.systemAll - cpuTime.systemAll;
		double stealDelta = newTime.steal - cpuTime.steal;
		double virtualDelta = newTime.virt - cpuTime.virt;
		double totalDelta = newTime.total - cpuTime.total;
		double usagePercent = (niceDelta + userDelta + systemAllDelta + stealDelta + virtualDelta) / totalDelta * (double)100.0;
		useStr = std::format("{:.2f}%", usagePercent);
	}
	cpuTime = newTime;
	return useStr;
}

void initCPULoadStatus()
{
	if(!Config::envIsLinux && !Config::envIsAndroid)
		return; // ignore cpu usage monitoring on non-linux systems
	if(procStatFile)
		return; // already open
	procStatFile = std::fopen("/proc/stat", "r");
	if(!procStatFile)
	{
		log.warn("can't open /proc/stat");
	}
}

void deinitCPULoadStatus()
{
	if(procStatFile)
	{
		std::fclose(procStatFile);
		procStatFile = nullptr;
	}
}

}
