REM Building C12Adapter libraries and examples
set BUILD_PATH=%~dp0%build
if not exist %BUILD_PATH% mkdir %BUILD_PATH%
cd %BUILD_PATH%
cmake ..
cmake --build . --config Debug
