cmake_minimum_required(VERSION 3.11)
project(reader)

set(METERINGSDK_COMPONENTS MCORE MCOM)
include(../../../src/MeteringSDK/MeteringSDK.cmake)

add_executable("reader" "reader.cpp" "Setup.cpp")
target_link_libraries("reader" MCORE MCOM)
