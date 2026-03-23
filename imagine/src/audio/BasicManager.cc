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

#include <imagine/config/macros.h>
#include <imagine/audio/Manager.hh>
#include <imagine/logger/SystemLogger.hh>

namespace IG::Audio
{

static SystemLogger log{"AudioManager"};

SampleFormat Manager::nativeSampleFormat() const
{
	return SampleFormats::f32;
}

int Manager::nativeRate() const
{
	return ::Config::MACHINE_IS_PANDORA ? 44100 : 48000;
}

Format Manager::nativeFormat() const
{
	return {nativeRate(), nativeSampleFormat(), 2};
}

void Manager::setSoloMix(std::optional<bool>) {}

bool Manager::soloMix() const { return false; }

void Manager::setMusicVolumeControlHint() {}

void Manager::startSession() {}

void Manager::endSession() {}

constexpr std::array<ApiDesc, systemApis.size()> apiDesc
{
	#ifdef CONFIG_PACKAGE_PULSEAUDIO
	ApiDesc{"PulseAudio", Api::PULSEAUDIO},
	#endif
	#ifdef CONFIG_PACKAGE_ALSA
	ApiDesc{"ALSA", Api::ALSA},
	#endif
};

std::vector<ApiDesc> Manager::audioAPIs() const
{
	return {std::begin(apiDesc), std::end(apiDesc)};
}

Api Manager::makeValidAPI(Api api) const
{
	for(auto desc: apiDesc)
	{
		if(desc.api == api)
		{
			log.debug("found requested API:{}", desc.name);
			return api;
		}
	}
	// API not found, use the default
	return apiDesc[0].api;
}

}
