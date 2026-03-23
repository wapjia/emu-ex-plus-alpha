addPkgLibarchive(imagine)

target_sources(imagine PRIVATE PosixIO.cc PosixFileIO.cc IO.cc MapIO.cc ArchiveIO.cc)
target_sources(imagine PRIVATE FILE_SET CXX_MODULES FILES internal.cc)

if(ENV STREQUAL android)
	target_sources(imagine PRIVATE AAssetIO.cc)
endif()
