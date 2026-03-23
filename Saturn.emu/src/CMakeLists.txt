target_sources(saturnemu PRIVATE FILE_SET CXX_MODULES
	BASE_DIRS main
	FILES main/system.ccm
)

target_sources(saturnemu PRIVATE
	main/Main.cc
	main/AppMeta.cc
	main/options.cc
	main/input.cc
	main/EmuMenuViews.cc
	ss/ak93c45.cpp
	ss/cdb.cpp
	ss/scu_dsp_gen.cpp
	ss/scu_dsp_misc.cpp
	ss/smpc.cpp
	ss/ss.cpp
	ss/stvio.cpp
	ss/vdp1_line.cpp
	ss/vdp1_sprite.cpp
	ss/vdp2_render.cpp
	ss/cart.cpp
	ss/db.cpp
	ss/scu_dsp_jmp.cpp
	ss/scu_dsp_mvi.cpp
	ss/sound.cpp
	ss/vdp1.cpp
	ss/vdp1_poly.cpp
	ss/vdp2.cpp
	ss/cart/ar4mp.cpp
	ss/cart/backup.cpp
	ss/cart/cs1ram.cpp
	ss/cart/bootrom.cpp
	ss/cart/extram.cpp
	ss/cart/rom.cpp
	ss/cart/stv.cpp
	ss/input/3dpad.cpp
	ss/input/gamepad.cpp
	ss/input/gun.cpp
	ss/input/jpkeyboard.cpp
	ss/input/keyboard.cpp
	ss/input/mission.cpp
	ss/input/mouse.cpp
	ss/input/multitap.cpp
	ss/input/wheel.cpp
)