C12Adapter
==========

This work is an open source version of a set of libraries developed at Elster Solutions.
Published under permission under The MIT License, see LICENSE.txt for details.

Use **build.bat** or **build.sh** to compile the libraries and the examples.
To cross-compile, depending on your environment, specify cmake toolchain such as:
```
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../src/MeteringSDK/CMake/Toolchains/arm-linux-moxa.cmake
cmake --build . --config Debug
```
You might have to edit toolchain file or create your own per your cross compiling environment.

API Reference: https://ElsterSolutionsOpensource.github.io/C12Adapter

To generate API reference install doxygen and say from the top level directory of the project: **doxygen Doxyfile**
