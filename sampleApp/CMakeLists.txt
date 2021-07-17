cmake_minimum_required(VERSION 3.5)
project(sampleApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DEP_ROOT ~/workspace)

set(SOURCE_FILES
	main.cpp
  )

add_definitions(-DBOOST_LOG_STATIC_LINK)
set(Boost_USE_STATIC_LIBS ON)

include_directories(.)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ogamma-sdk/include)
include_directories(${CONAN_INCLUDE_DIRS})

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ogamma-sdk/lib)
link_directories(${CONAN_LIB_DIRS})

add_executable(sampleApp ${SOURCE_FILES})

if (WIN32)
	target_link_libraries (sampleApp OpcUaSdk ${CONAN_LIBS})
	add_compile_options(/bigobj)
else()
	target_link_libraries (sampleApp OpcUaSdk ${CONAN_LIBS} ${CMAKE_DL_LIBS})
endif()	


