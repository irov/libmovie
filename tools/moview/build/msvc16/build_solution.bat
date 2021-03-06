@echo off

set "PROJECT=moview"
set "CONFIGURATION=%1"
set "SOLUTION_TYPE=MSVC"
set "SOLUTION_VERSION=16"
set "SOLUTION_DIR=..\solutions\%PROJECT%_%SOLUTION_TYPE%%SOLUTION_VERSION%_%CONFIGURATION%"

@pushd ..
@mkdir %SOLUTION_DIR%
@pushd %SOLUTION_DIR%
CMake -G "Visual Studio %SOLUTION_VERSION% 2019" -A Win32 "%CD%\..\.." -DCMAKE_BUILD_TYPE:STRING=%CONFIGURATION% -D%SOLUTION_TYPE%:BOOLEAN=ON
CMake --build . --config %CONFIGURATION%
@popd
@popd

@echo on
@pause
