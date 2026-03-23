target_sources(nesemu PRIVATE FILE_SET CXX_MODULES
	BASE_DIRS main
	FILES main/system.ccm main/io.ccm
)

file(GLOB boardSrcs "fceu/boards/*.cpp" "fceu/boards/*.c")

target_sources(nesemu PRIVATE
	main/Main.cc
	main/AppMeta.cc
	main/input.cc
	main/options.cc
	main/EmuMenuViews.cc
	main/FceuApi.cc
	main/Cheats.cc
	fceu/cart.cpp
	fceu/cheat.cpp
	fceu/emufile.cpp
	fceu/fceu.cpp
	fceu/file.cpp
	fceu/filter.cpp
	fceu/ines.cpp
	fceu/input.cpp
	fceu/nsf.cpp
	fceu/palette.cpp
	fceu/ppu.cpp
	fceu/sound.cpp
	fceu/state.cpp
	fceu/unif.cpp
	fceu/vsuni.cpp
	fceu/x6502.cpp
	fceu/movie.cpp
	fceu/fds.cpp
	fceu/utils/crc32.cpp
	fceu/utils/md5.cpp
	fceu/utils/memory.cpp
	fceu/utils/xstring.cpp
	fceu/utils/endian.cpp
	fceu/utils/general.cpp
	fceu/utils/guid.cpp
	fceu/input/mouse.cpp
	fceu/input/oekakids.cpp
	fceu/input/powerpad.cpp
	fceu/input/quiz.cpp
	fceu/input/shadow.cpp
	fceu/input/suborkb.cpp
	fceu/input/toprider.cpp
	fceu/input/zapper.cpp
	fceu/input/bworld.cpp
	fceu/input/arkanoid.cpp
	fceu/input/mahjong.cpp
	fceu/input/fkb.cpp
	fceu/input/ftrainer.cpp
	fceu/input/hypershot.cpp
	fceu/input/cursor.cpp
	fceu/input/snesmouse.cpp
	fceu/input/fns.cpp
	fceu/input/pec586kb.cpp
	fceu/input/virtualboy.cpp
	fceu/input/lcdcompzapper.cpp
	${boardSrcs}
)
