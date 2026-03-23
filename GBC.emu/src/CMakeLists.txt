target_sources(gbcemu PRIVATE FILE_SET CXX_MODULES
	BASE_DIRS main
	FILES main/system.ccm
)

target_sources(gbcemu PRIVATE
	main/Main.cc
	main/AppMeta.cc
	main/options.cc
	main/EmuMenuViews.cc
	main/Cheats.cc
	main/Palette.cc
	libgambatte/src/cpu.cpp
	libgambatte/src/gambatte.cpp
	libgambatte/src/initstate.cpp
	libgambatte/src/interrupter.cpp
	libgambatte/src/tima.cpp
	libgambatte/src/memory.cpp
	libgambatte/src/mem/rtc.cpp
	libgambatte/src/sound.cpp
	libgambatte/src/statesaver.cpp
	libgambatte/src/video.cpp
	libgambatte/src/sound/channel1.cpp
	libgambatte/src/sound/channel2.cpp
	libgambatte/src/sound/channel3.cpp
	libgambatte/src/sound/channel4.cpp
	libgambatte/src/sound/duty_unit.cpp
	libgambatte/src/sound/envelope_unit.cpp
	libgambatte/src/sound/length_counter.cpp
	libgambatte/src/video/ly_counter.cpp
	libgambatte/src/video/lyc_irq.cpp
	libgambatte/src/video/next_m0_time.cpp
	libgambatte/src/video/ppu.cpp
	libgambatte/src/video/sprite_mapper.cpp
	libgambatte/src/mem/cartridge.cpp
	libgambatte/src/mem/memptrs.cpp
	libgambatte/src/interruptrequester.cpp
	libgambatte/src/mem/pakinfo.cpp
	libgambatte/src/loadres.cpp
	common/resample/src/resamplerinfo.cpp
	common/resample/src/makesinckernel.cpp
	common/resample/src/chainresampler.cpp
	common/resample/src/u48div.cpp
	common/resample/src/i0.cpp
	common/resample/src/kaiser50sinc.cpp
	common/resample/src/kaiser70sinc.cpp
)