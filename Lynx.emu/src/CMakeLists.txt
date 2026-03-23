target_sources(lynxemu PRIVATE FILE_SET CXX_MODULES
	BASE_DIRS main
	FILES main/system.ccm
)

target_sources(lynxemu PRIVATE
	main/Main.cc
	main/AppMeta.cc
	main/options.cc
	main/input.cc
	main/EmuMenuViews.cc
	lynx/c65c02.cpp
	lynx/cart.cpp
	lynx/lynxdec.cpp
	lynx/memmap.cpp
	lynx/mikie.cpp
	lynx/ram.cpp
	lynx/rom.cpp
	lynx/susie.cpp
	lynx/system.cpp
)