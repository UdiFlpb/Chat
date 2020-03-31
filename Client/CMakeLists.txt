cmake_minimum_required(VERSION 2.8.9)
project(Chat)
include_directories(include)
get_filename_component(DIR_ONE_ABOVE ../ ABSOLUTE)
message(STATUS ${DIR_ONE_ABOVE})
include_directories(${DIR_ONE_ABOVE}/Shared)
set(SOURCES src/main.cpp)
add_executable(Client ${SOURCES})






