mkdir build
cd build

cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release ^
       -DCMAKE_PREFIX_PATH=%LIBRARY_PREFIX% ^
       -DCMAKE_INSTALL_PREFIX=%SP_DIR%/ngsolve ^
       -DCMAKE_SYSTEM_PREFIX_PATH=%SYS_PREFIX% \
       %SRC_DIR%
if errorlevel 1 exit 1

REM Build
nmake
if errorlevel 1 exit 1

REM Install
nmake install
if errorlevel 1 exit 1
