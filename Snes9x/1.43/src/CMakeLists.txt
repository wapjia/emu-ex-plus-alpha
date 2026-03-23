target_sources(s9x PRIVATE FILE_SET CXX_MODULES
	BASE_DIRS main
	FILES main/system.ccm
)

target_sources(s9x PRIVATE
	main/Main.cc
	main/AppMeta.cc
	main/input.cc
	main/options.cc
	main/S9XApi.cc
	main/EmuMenuViews.cc
	main/Cheats.cc
	snes9x-1.43/c4.cpp
	snes9x-1.43/c4emu.cpp
	snes9x-1.43/cheats.cpp
	snes9x-1.43/cheats2.cpp
	snes9x-1.43/clip.cpp
	snes9x-1.43/cpu.cpp
	snes9x-1.43/cpuexec.cpp
	snes9x-1.43/cpuops.cpp
	snes9x-1.43/dma.cpp
	snes9x-1.43/dsp1.cpp
	snes9x-1.43/data.cpp
	snes9x-1.43/fxemu.cpp
	snes9x-1.43/fxinst.cpp
	snes9x-1.43/gfx.cpp
	snes9x-1.43/globals.cpp
	snes9x-1.43/loadzip.cpp
	snes9x-1.43/memmap.cpp
	snes9x-1.43/movie.cpp
	snes9x-1.43/obc1.cpp
	snes9x-1.43/ppu.cpp
	snes9x-1.43/sa1.cpp
	snes9x-1.43/sa1cpu.cpp
	snes9x-1.43/sdd1.cpp
	snes9x-1.43/sdd1emu.cpp
	snes9x-1.43/seta.cpp
	snes9x-1.43/seta010.cpp
	snes9x-1.43/seta011.cpp
	snes9x-1.43/seta018.cpp
	snes9x-1.43/snapshot.cpp
	snes9x-1.43/spc7110.cpp
	snes9x-1.43/srtc.cpp
	snes9x-1.43/tile.cpp
	snes9x-1.43/spc700.cpp
	snes9x-1.43/soundux.cpp
	snes9x-1.43/apu.cpp
)