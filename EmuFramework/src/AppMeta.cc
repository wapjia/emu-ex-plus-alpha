/*  This file is part of EmuFramework.

	EmuFramework is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	EmuFramework is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with EmuFramework.  If not, see <http://www.gnu.org/licenses/> */

#include <emuframework/AppMeta.hh>

namespace EmuEx
{

[[gnu::weak]] const int AppMeta::maxPlayers{1};
[[gnu::weak]] const bool AppMeta::inputHasKeyboard{};
[[gnu::weak]] const bool AppMeta::hasPALVideoSystem{};
[[gnu::weak]] const bool AppMeta::canRenderRGB565{true};
[[gnu::weak]] const bool AppMeta::canRenderRGBA8888{true};
[[gnu::weak]] const bool AppMeta::hasRectangularPixels{};
[[gnu::weak]] const AspectRatioInfo AppMeta::aspectRatioInfo{"4:3 (Original)", {4, 3}};
[[gnu::weak]] const bool AppMeta::hasResetModes{};
[[gnu::weak]] const bool AppMeta::handlesArchiveFiles{};
[[gnu::weak]] const bool AppMeta::handlesGenericIO{true};
[[gnu::weak]] const bool AppMeta::hasCheats{};
[[gnu::weak]] const bool AppMeta::stateSizeChangesAtRuntime{};
[[gnu::weak]] const bool AppMeta::hasSound{true};
[[gnu::weak]] const int AppMeta::forcedSoundRate{};
[[gnu::weak]] const Audio::SampleFormat AppMeta::audioSampleFormat{Audio::SampleFormats::i16};
[[gnu::weak]] const F2Size AppMeta::validFrameRateRange{minFrameRate, 80.};
[[gnu::weak]] const bool AppMeta::hasIcon{true};
[[gnu::weak]] const bool AppMeta::needsGlobalInstance{};
[[gnu::weak]] const bool AppMeta::handlesRecentContent{};
[[gnu::weak]] const std::span<const BundledGameInfo> AppMeta::bundledGameInfo{};

[[gnu::weak]] bool AppMeta::allowsTurboModifier(KeyCode) { return true; }
[[gnu::weak]] std::unique_ptr<View> AppMeta::makeEditCheatsView(ViewAttachParams, CheatsView&) { return {}; }
[[gnu::weak]] std::unique_ptr<View> AppMeta::makeEditCheatView(ViewAttachParams, Cheat&, BaseEditCheatsView&) { return {}; }
[[gnu::weak]] void AppMeta::onCustomizeNavView(AppNavView&) {}

}
