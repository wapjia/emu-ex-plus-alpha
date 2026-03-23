/*  This file is part of 2600.emu.

	2600.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	2600.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with 2600.emu.  If not, see <http://www.gnu.org/licenses/> */

module;
#include <stella/emucore/Cart.hxx>
#include <stella/emucore/CartCreator.hxx>
#include <stella/emucore/MD5.hxx>
#include <stella/emucore/Sound.hxx>
#include <stella/emucore/tia/TIA.hxx>
#include <stella/emucore/Switches.hxx>
#include <stella/emucore/Paddles.hxx>
#include <stella/emucore/PropsSet.hxx>
#include <stella/emucore/M6532.hxx>
#include <stella/emucore/DispatchResult.hxx>
#include <stella/common/StateManager.hxx>
#include <stella/common/AudioSettings.hxx>
#include <EventHandler.hxx>
#include <FrameBuffer.hxx>
#include <SoundEmuEx.hh>

module system;

namespace EmuEx
{

constexpr size_t MAX_ROM_SIZE = 512 * 1024;

extern "C++" std::string_view EmuSystem::shortSystemName() const { return "2600"; }
extern "C++" std::string_view EmuSystem::systemName() const { return "Atari 2600"; }

FS::FileString A2600System::stateFilename(int slot, std::string_view name) const
{
	return format<FS::FileString>("{}.0{}.sta", name, saveSlotChar(slot));
}

void A2600System::closeSystem()
{
	osystem.deleteConsole();
}

void A2600System::updateSwitchValues()
{
	auto switches = osystem.console().switches().read();
	log.info("updating switch values to {:X}", switches);
	p1DiffB = !(switches & 0x40);
	p2DiffB = !(switches & 0x80);
	vcsColor = switches & 0x08;
}

VideoSystem A2600System::videoSystem() const
{
	return osystem.hasConsole()
		&& osystem.console().timing() != ConsoleTiming::ntsc ? VideoSystem::PAL : VideoSystem::NATIVE_NTSC;
}

void A2600System::loadContent(IO &io, EmuSystemCreateParams, OnLoadProgressDelegate)
{
	auto &os = osystem;
	if(io.size() > MAX_ROM_SIZE)
		throw std::runtime_error{"ROM size is too large"};
	auto image = std::make_unique<uInt8[]>(MAX_ROM_SIZE);
	auto size = io.read(image.get(), MAX_ROM_SIZE);
	if(size == -1)
	{
		throwFileReadError();
	}
	string md5 = MD5::hash(image, size);
	Properties props{};
	os.propSet().getMD5(md5, props);
	defaultGameProps = props;
	auto &romType = props.get(PropType::Cart_Type);
	FSNode fsNode{contentFileName().data()};
	auto &settings = os.settings();
	settings.setValue("romloadcount", 0);
	settings.setValue("plr.tv.jitter", false);
	auto cartridge = CartCreator::create(fsNode, image, size, md5, romType, settings);
	cartridge->setMessageCallback([](const string& msg){ log.info("{}", msg); });
	if((int)optionTVPhosphor != TV_PHOSPHOR_AUTO)
	{
		props.set(PropType::Display_Phosphor, optionTVPhosphor ? "YES" : "NO");
	}
	os.frameBuffer().enablePhosphor(props.get(PropType::Display_Phosphor) == "YES", optionTVPhosphorBlend);
	if((int)optionVideoSystem) // not auto
	{
		log.info("forcing video system to:{}", optionVideoSystemToStr(optionVideoSystem));
		props.set(PropType::Display_Format, optionVideoSystemToStr(optionVideoSystem));
	}
	os.makeConsole(cartridge, props, contentFileName().data());
	auto &console = os.console();
	autoDetectedInput1 = limitToSupportedControllerTypes(console.leftController().type());
	setControllerType(EmuApp::get(appContext()), console, optionInputPort1);
	Paddles::setDigitalSensitivity(optionPaddleDigitalSensitivity);
	console.initializeVideo();
	console.initializeAudio();
	log.info("is PAL:{}", videoSystem() == VideoSystem::PAL ? "yes" : "no");
	Serializer state;
	osystem.state().saveState(state);
	saveStateSize = state.size();
}

static auto consoleFrameRate(const OSystem& osystem)
{
	if(!osystem.hasConsole())
		return 60.f;
	if(!osystem.console().tia().frameBufferScanlinesLastFrame())
		return osystem.console().timing() == ConsoleTiming::ntsc ? 60.f : 50.f;
	return osystem.console().currentFrameRate();
}

FrameRate A2600System::frameRate() const
{
	return consoleFrameRate(osystem);
}

void A2600System::configAudioRate(FrameRate outputFrameRate, int outputRate)
{
	if(!osystem.hasConsole())
		return;
	configuredInputVideoFrameRate = consoleFrameRate(osystem);
	osystem.setSoundMixRate(std::round(audioMixRate(outputRate, configuredInputVideoFrameRate, outputFrameRate)));
}

static void renderVideo(EmuSystemTaskContext taskCtx, EmuVideo& video, FrameBuffer& fb, TIA& tia)
{
	auto fmt = video.renderPixelFormat();
	auto img = video.startFrameWithFormat(taskCtx, {{(int)tia.width(), (int)tia.height()}, fmt});
	fb.render(img.pixmap(), tia);
	img.endFrame();
}

void A2600System::runFrame(EmuSystemTaskContext taskCtx, EmuVideo* video, EmuAudio* audio)
{
	auto &os = osystem;
	auto &console = os.console();
	auto &sound = os.soundEmuEx();
	sound.setEmuAudio(audio);
	console.riot().update();
	auto &tia = console.tia();
	static constexpr uInt64 maxCyclesPerFrame = 32768;
	DispatchResult res;
	tia.update(res, maxCyclesPerFrame);
	if(res.getCycles() > maxCyclesPerFrame)
		log.warn("frame ran {} cycles", res.getCycles());
	tia.renderToFrameBuffer();
	if(video)
	{
		renderVideo(taskCtx, *video, os.frameBuffer(), tia);
	}
	if(auto newInputVideoFrameRate = osystem.console().currentFrameRate();
		configuredInputVideoFrameRate != newInputVideoFrameRate
		&& newInputVideoFrameRate >= 40.0f && newInputVideoFrameRate <= 70.0f) [[unlikely]]
	{
		onFrameRateChanged();
	}
}

void A2600System::renderFramebuffer(EmuVideo& video)
{
	auto &tia = osystem.console().tia();
	auto &fb = osystem.frameBuffer();
	renderVideo({}, video, fb, tia);
}

void A2600System::reset(EmuApp&, ResetMode mode)
{
	assume(hasContent());
	if(mode == ResetMode::HARD)
	{
		osystem.console().system().reset();
	}
	else
	{
		Event &ev = osystem.eventHandler().event();
		ev.clear();
		ev.set(Event::ConsoleReset, 1);
		auto &console = osystem.console();
		console.switches().update();
		TIA& tia = console.tia();
		tia.update(console.emulationTiming().cyclesPerFrame());
		ev.set(Event::ConsoleReset, 0);
	}
}

void A2600System::readState(EmuApp& app, std::span<uint8_t> buff)
{
	Serializer state;
	state.putByteArray(buff.data(), buff.size());
	if(!osystem.state().loadState(state))
		throw std::runtime_error("Invalid state data");
	updateSwitchValues();
}

size_t A2600System::writeState(std::span<uint8_t> buff, SaveStateFlags flags)
{
	Serializer state;
	osystem.state().saveState(state);
	assume(state.size() == saveStateSize);
	assume(state.size() <= buff.size());
	state.getByteArray(buff.data(), buff.size());
	return saveStateSize;
}

bool A2600System::onVideoRenderFormatChange(EmuVideo&, PixelFormat fmt)
{
	osystem.frameBuffer().setPixelFormat(fmt);
	if(osystem.hasConsole())
	{
		osystem.frameBuffer().paletteHandler().setPalette();
	}
	return false;
}

void A2600System::onOptionsLoaded()
{
	osystem.soundEmuEx().setResampleQuality(optionAudioResampleQuality);
}

}
