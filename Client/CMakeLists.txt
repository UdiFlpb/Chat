cmake_minimum_required(VERSION 2.8.9)
project(Chat)

include_directories(include)
include_directories(../Shared)

file(GLOB SOURCES "src/*.cpp")
add_executable(Client ${SOURCES})






