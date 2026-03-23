target_sources(mdemu PRIVATE FILE_SET CXX_MODULES
	BASE_DIRS main
	FILES main/system.ccm
)

target_sources(mdemu PRIVATE
	main/Main.cc
	main/AppMeta.cc
	main/options.cc
	main/input.cc
	main/EmuMenuViews.cc
	main/Cheats.cc
	genplus-gx/system.cc
	genplus-gx/genesis.cc
	genplus-gx/io_ctrl.cc
	genplus-gx/loadrom.cc
	genplus-gx/mem68k.cc
	genplus-gx/membnk.cc
	genplus-gx/memz80.cc
	genplus-gx/state.cc
	genplus-gx/vdp_ctrl.cc
	genplus-gx/vdp_render.cc
	genplus-gx/m68k/musashi/m68kcpu.cc
	genplus-gx/z80/z80.cc
	genplus-gx/sound/sound.cc
	genplus-gx/sound/ym2612.cc
	genplus-gx/sound/Fir_Resampler.cc
	genplus-gx/sound/ym2413.cc
	genplus-gx/sound/sn76489.cc
	genplus-gx/sound/blip.cc
	genplus-gx/cart_hw/eeprom.cc
	genplus-gx/cart_hw/areplay.cc
	genplus-gx/cart_hw/ggenie.cc
	genplus-gx/cart_hw/md_cart.cc
	genplus-gx/cart_hw/sram.cc
	genplus-gx/cart_hw/svp/svp.cc
	genplus-gx/cart_hw/svp/ssp16.cc
	genplus-gx/cart_hw/sms_cart.cc
	genplus-gx/input_hw/input.cc
	genplus-gx/input_hw/activator.cc
	genplus-gx/input_hw/gamepad.cc
	genplus-gx/input_hw/lightgun.cc
	genplus-gx/input_hw/mouse.cc
	genplus-gx/input_hw/teamplayer.cc
	genplus-gx/input_hw/xe_a1p.cc
	genplus-gx/input_hw/sportspad.cc
	genplus-gx/input_hw/paddle.cc
	scd/scd.cc
	scd/LC89510.cc
	scd/cd_sys.cc
	scd/gfx_cd.cc
	scd/pcm.cc
	scd/cd_file.cc
	scd/memMain.cc
	scd/memSub.cc
)