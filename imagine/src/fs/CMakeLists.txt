target_sources(imagine PRIVATE FS.cc PosixFS.cc ArchiveFS.cc)

if(ENV STREQUAL android)
	target_sources(imagine PRIVATE AAssetFS.cc)
endif()
