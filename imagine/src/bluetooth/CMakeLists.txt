if(${ENV} STREQUAL linux)
	addPkgBluez(imagine)
	target_sources(imagine PRIVATE BluezBluetoothAdapter.cc)
elseif(${ENV} STREQUAL android)
	target_sources(imagine PRIVATE AndroidBluetoothAdapter.cc)
elseif(${ENV} STREQUAL ios AND NOT ARCH STREQUAL x86)
	addPkgBtStack(imagine)
	target_sources(imagine PRIVATE BtstackBluetoothAdapter.cc)
endif()

target_sources(
	imagine PRIVATE
	BluetoothInputDevScanner.cc
	Wiimote.cc
	IControlPad.cc
	Zeemote.cc
)

if(ENV STREQUAL linux OR ENV STREQUAL ios)
	target_sources(imagine PRIVATE PS3Controller.cc)
endif()
