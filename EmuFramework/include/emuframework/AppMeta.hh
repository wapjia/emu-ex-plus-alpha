#pragma once

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

#include <emuframework/EmuInput.hh>
#ifdef IG_USE_MODULES
import imagine;
import std;
#else
#include <imagine/audio/Format.hh>
#include <imagine/gui/NavView.hh>
#include <span>
#include <string_view>
#include <memory>
#endif

namespace EmuEx
{

class CheatsView;
class Cheat;
class BaseEditCheatsView;
struct AssetDesc;

struct AppMeta
{
	// Static app configuration
	static const int maxPlayers;
	static const bool inputHasKeyboard;
	static const bool hasPALVideoSystem;
	static const bool canRenderRGB565;
	static const bool canRenderRGBA8888;
	static const bool hasRectangularPixels;
	static const AspectRatioInfo aspectRatioInfo;
	static const bool hasResetModes;
	static const bool handlesArchiveFiles;
	static const bool handlesGenericIO;
	static const bool hasCheats;
	static const bool hasSound;
	static const int forcedSoundRate;
	static const Audio::SampleFormat audioSampleFormat;
	static constexpr double minFrameRate{48.};
	static const F2Size validFrameRateRange;
	static const bool stateSizeChangesAtRuntime;
	static const bool hasIcon;
	static const bool needsGlobalInstance;
	static const bool handlesRecentContent;
	static const std::u16string_view mainViewName;
	static const std::string_view creditsViewStr;
	static const std::string_view configFilename;
	static const bool hasGooglePlayStoreFeatures;
	static const NameFilterFunc defaultFsFilter;
	static const std::span<const BundledGameInfo> bundledGameInfo;

	static bool hasBundledGames() { return bundledGameInfo.size(); }

	// Input
	static std::span<const KeyCategory> keyCategories();
	static std::span<const KeyConfigDesc> defaultKeyConfigs();
	static std::string_view systemKeyCodeToString(KeyCode);
	static bool allowsTurboModifier(KeyCode);
	static AssetDesc vControllerAssetDesc(KeyInfo);
	static SystemInputDeviceDesc inputDeviceDesc(int idx);

	// Cheats
	static std::unique_ptr<View> makeEditCheatsView(ViewAttachParams, CheatsView&);
	static std::unique_ptr<View> makeEditCheatView(ViewAttachParams, Cheat&, BaseEditCheatsView&);

	// GUI
	static void onCustomizeNavView(AppNavView&);
};

}
