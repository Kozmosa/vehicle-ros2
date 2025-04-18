cmake_minimum_required(VERSION 3.20)
project(project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)

include_directories(./thirdparty/serialib/ )

add_subdirectory(thirdparty/serialib)

aux_source_directory(./src SOURCE)

add_executable(project ${SOURCE})

target_link_libraries(project serialib)