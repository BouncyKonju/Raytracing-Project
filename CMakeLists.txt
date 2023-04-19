cmake_minimum_required(VERSION 3.0.0)
project(raytracing VERSION 0.1.0)

include(CTest)
enable_testing()

file(GLOB sourceFiles CMAKE_CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(TO_CMAKE_PATH $ENV{OPENCV_LIB} OPENCVLIB)
file(TO_CMAKE_PATH $ENV{OPENCV_BIN} OPENCVBIN)

find_package(OpenCV REQUIRED)
find_library(OpenCL OpenCL lib/OpenCL/lib)

add_executable(raytracing main.cpp)

add_library(source ${sourceFiles})

find_package(OpenCV)

target_link_libraries(raytracing ${OpenCL})
target_link_libraries(raytracing ${OpenCV_LIBS})
target_link_libraries(raytracing source)
target_include_directories(source PUBLIC ${CMAKE_SOURCE_DIR}/include/)
include_directories(${CMAKE_SOURCE_DIR}/include lib lib/OpenCL/include ${OpenCV_DIRS})
target_include_directories(source PUBLIC ${CMAKE_SOURCE_DIR}/include)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CMAKE_CXX_FLAGS "-std=c++17 -pthread -w -O3 -ffast-math -lopencv_core -lopencv_highgui")
include(CPack)
