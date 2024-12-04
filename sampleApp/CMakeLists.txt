cmake_minimum_required(VERSION 3.5)
project(sampleApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_FILES
	main.cpp
  )

add_executable(sampleApp ${SOURCE_FILES})

if (WIN32)
	target_link_libraries (sampleApp OpcUaSdk ${OGAMMA_SDK_LIBS})
	add_compile_options(/bigobj)
else()
	target_link_libraries (sampleApp OpcUaSdk ${OGAMMA_SDK_LIBS} ${CMAKE_DL_LIBS})
endif()	
