ifndef inc_main
inc_main := 1

include $(IMAGINE_PATH)/make/imagineAppBase.mk

CPPFLAGS += -I$(projectPath)/src \
-DEMU_EX_PLATFORM \
-DSOUND_SUPPORT \
-DTHUMB_SUPPORT \
-I$(projectPath)/src/stella/emucore \
-I$(projectPath)/src/stella/emucore/tia \
-I$(projectPath)/src/stella/emucore/elf \
-I$(projectPath)/src/stella/common \
-I$(projectPath)/src/stella/common/tv_filters \
-I$(projectPath)/src/stella/gui

CFLAGS_WARN += -Wno-unused-parameter -Wno-deprecated-literal-operator

stellaSrc := AtariVox.cxx \
Bankswitch.cxx \
Booster.cxx \
CompuMate.cxx \
Console.cxx \
Control.cxx \
ControllerDetector.cxx \
CortexM0.cxx \
DispatchResult.cxx \
Driving.cxx \
EmulationTiming.cxx \
Genesis.cxx \
Joy2BPlus.cxx \
Joystick.cxx \
Keyboard.cxx \
KidVid.cxx \
Lightgun.cxx \
M6502.cxx \
M6532.cxx \
MD5.cxx \
MindLink.cxx \
MT24LC256.cxx \
Paddles.cxx \
PlusROM.cxx \
PointingDevice.cxx \
Props.cxx \
PropsSet.cxx \
QuadTari.cxx \
SaveKey.cxx \
Serializer.cxx \
Settings.cxx \
Switches.cxx \
System.cxx \
Thumbulator.cxx \
tia/AnalogReadout.cxx \
tia/Audio.cxx \
tia/AudioChannel.cxx \
tia/Background.cxx \
tia/Ball.cxx \
tia/DrawCounterDecodes.cxx \
tia/LatchedInput.cxx \
tia/Missile.cxx \
tia/Player.cxx \
tia/Playfield.cxx \
tia/TIA.cxx \
tia/frame-manager/AbstractFrameManager.cxx \
tia/frame-manager/FrameLayoutDetector.cxx \
tia/frame-manager/FrameManager.cxx \
tia/frame-manager/JitterEmulation.cxx \
elf/BusTransactionQueue.cxx \
elf/ElfEnvironment.cxx \
elf/ElfLinker.cxx \
elf/ElfParser.cxx \
elf/ElfUtil.cxx \
elf/VcsLib.cxx

cartsSrc := $(subst $(projectPath)/src/,,$(wildcard $(projectPath)/src/stella/emucore/Cart*.cxx))
stellaPath := stella/emucore
SRC += main/Main.cc \
main/options.cc \
main/input.cc \
main/EmuMenuViews.cc \
main/SoundEmuEx.cc \
main/FrameBuffer.cc \
main/OSystem.cc \
main/FSNodeEmuEx.cc \
stella/common/AudioQueue.cxx \
stella/common/AudioSettings.cxx \
stella/common/Base.cxx \
stella/common/DevSettingsHandler.cxx \
stella/common/PaletteHandler.cxx \
stella/common/RewindManager.cxx \
stella/common/StateManager.cxx \
stella/common/TimerManager.cxx \
stella/common/audio/ConvolutionBuffer.cxx \
stella/common/audio/HighPass.cxx \
stella/common/audio/LanczosResampler.cxx \
stella/common/audio/SimpleResampler.cxx \
stella/common/repository/CompositeKeyValueRepository.cxx \
$(cartsSrc) \
$(addprefix $(stellaPath)/,$(stellaSrc))

include $(EMUFRAMEWORK_PATH)/package/emuframework.mk

include $(IMAGINE_PATH)/make/imagineAppTarget.mk

endif
