# Support of ARM Linux toolchain from MOXA

SET(CMAKE_SYSTEM_NAME Linux)
SET(UNIX ON)
SET(ARM_LINUX_MOXA ON)

SET(CROSS_COMPILER_PREFIX arm-linux-gnueabihf-)

find_path(CROSS_COMPILER_PATH bin/${CROSS_COMPILER_PREFIX}gcc
    PATHS /usr/local/arm-linux-gnueabihf
          /usr/local/arm-linux-gnueabihf-4.7-20130415
          /usr/arm-linux-gnueabihf
          /usr/arm-linux-gnueabihf-4.7-20130415
          /opt/local/arm-linux-gnueabihf
          /opt/local/arm-linux-gnueabihf-4.7-20130415
          /opt/arm-linux-gnueabihf
          /opt/arm-linux-gnueabihf-4.7-20130415
          /usr/local
          /usr
    NO_DEFAULT_PATH)
if(NOT CROSS_COMPILER_PATH)
    message(FATAL_ERROR "MOXA toolchain not found!")
endif()

SET(CMAKE_C_COMPILER "${CROSS_COMPILER_PATH}/bin/${CROSS_COMPILER_PREFIX}gcc")
SET(CMAKE_CXX_COMPILER "${CROSS_COMPILER_PATH}/bin/${CROSS_COMPILER_PREFIX}g++")

SET(CMAKE_FIND_ROOT_PATH "${CROSS_COMPILER_PATH}")

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
