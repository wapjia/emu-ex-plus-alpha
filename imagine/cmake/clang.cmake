if(NOT CMAKE_AR)
	set(CMAKE_AR llvm-ar)
endif()

# needed for DelegateFuncSet.hh
string(APPEND CXXFLAGS " -Wno-vla-cxx-extension")

include("${CMAKE_CURRENT_LIST_DIR}/compiler-common.cmake")
