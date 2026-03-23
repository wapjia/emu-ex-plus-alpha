if(ENV STREQUAL android)
	addPkgConfigLibs(imagine "-llog")
endif()

target_sources(imagine PRIVATE logger.cc)
