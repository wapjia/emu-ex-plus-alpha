addPkgConfigLibs(imagine "-framework=AudioToolbox -framework=AVFoundation")
target_sources(imagine PRIVATE coreaudio.cc AvfManager.mm)
