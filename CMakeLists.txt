cmake_minimum_required(VERSION 3.15)
project(sampleApp)

set(OGAMMA_SDK_PATH ${CMAKE_SOURCE_DIR}/../ogamma-sdk)
if (WIN32)
	set(OGAMMA_SDK_LIBS
		OpcUaSdk.lib
		botan.lib
		spdlog.lib
		pugixml.lib
		fmt.lib
		libboost_filesystem.lib
	)
else()
	set(OGAMMA_SDK_LIBS
		libOpcUaSdk.a
		libbotan-2.a
		libspdlog.a
		libpugixml.a
		libfmt.a
		libboost_filesystem.a
	)
endif()

include_directories(${OGAMMA_SDK_PATH}/include)

if (WIN32)
	add_compile_options(/bigobj)
	link_directories(${OGAMMA_SDK_PATH}/lib/x64/vs2022)
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")
	link_directories(${OGAMMA_SDK_PATH}/lib/x64/debian1010)
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")
	link_directories(${OGAMMA_SDK_PATH}/lib/x64/ubuntu)
endif()
add_subdirectory(sampleApp)
