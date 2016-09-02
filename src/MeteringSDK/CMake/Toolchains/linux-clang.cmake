# File linux-clang.cmake
#
# LLVM cmake toolchain file. Usage:
#
#   cmake -DCMAKE_TOOLCHAIN_FILE=~/project/directory/here/src/MeteringSDK/CMake/Toolchains/linux-clang.cmake  ~/your/directory/here
#
# Some info from is not used currently: http://mhungerford.blogspot.ru/2010/10/cmake-and-clangllvm-fun.html

#INCLUDE(CMakeForceCompiler)    # Disable some checks
#SET(CMAKE_SYSTEM_NAME Generic) # Generic removes -rdynamic from linker, which llvm-ld doesn't support

#CMAKE_FORCE_C_COMPILER  (/usr/bin/clang   CLang)
#CMAKE_FORCE_CXX_COMPILER(/usr/bin/clang++ CLang)

#  @SET(CMAKE_RANLIB "/usr/bin/llvm-ranlib" CACHE INTERNAL STRING)
#  SET(CMAKE_AR     "/usr/bin/llvm-ar"     CACHE INTERNAL STRING)
#  SET(CMAKE_LINKER "/usr/bin/llvm-ld"     CACHE INTERNAL STRING)
#  #SET(CMAKE_C_LINKER "/usr/bin/llvm-ld")
#  #SET(CMAKE_CXX_LINKER "/usr/bin/llvm-ld")

#  SET(CMAKE_C_LINK_EXECUTABLE   "/usr/bin/llvm-ld <OBJECTS> -o <TARGET> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <LINK_LIBRARIES>")
#  SET(CMAKE_CXX_LINK_EXECUTABLE "/usr/bin/llvm-ld <OBJECTS> -o <TARGET> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <LINK_LIBRARIES>")

#  SET(CMAKE_FIND_ROOT_PATH /usr/bin)
#  SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
#  SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#  SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

SET(CMAKE_C_COMPILER   "/usr/bin/clang")
SET(CMAKE_CXX_COMPILER "/usr/bin/clang++")
SET(CLANG 1)
