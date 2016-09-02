cmake_minimum_required(VERSION 2.8)
project(c1222)

include(../../../src/MeteringSDK/MeteringSDK.cmake)

add_executable("c1222client" "c1222_client.cpp")
target_link_libraries("c1222client" MCORE MCOM)
