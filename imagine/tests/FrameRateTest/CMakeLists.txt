cmake_minimum_required(VERSION 4.1)

project(
	FrameRateTest
	DESCRIPTION "Frame Rate Test"
	HOMEPAGE_URL "https://www.explusalpha.com/"
)

printConfigInfo()
configureAppTarget(frameRateTest)
target_compile_options(frameRateTest PRIVATE -Werror)
addPkgConfigDepMultiConfig(frameRateTest imagine)
addCxxModules(frameRateTest imagine)
evalPkgConfigFlags(frameRateTest all)
target_sources(frameRateTest PRIVATE FILE_SET CXX_MODULES BASE_DIRS src/main FILES
	src/main/cpuUtils.ccm
	src/main/TestPicker.ccm
	src/main/tests.ccm
)
target_sources(frameRateTest PRIVATE
	src/main/main.cc
	src/main/cpuUtils.cc
	src/main/tests.cc
)
