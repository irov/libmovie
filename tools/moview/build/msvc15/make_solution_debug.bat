@echo off

set "PROJECT=moview"
set "CONFIGURATION=Debug"
set "SOLUTION_DIR=..\solutions\%PROJECT%_msvc15_%CONFIGURATION%"

@pushd ..
@mkdir %SOLUTION_DIR%
@pushd %SOLUTION_DIR%
CMake -G "Visual Studio 15 2017" "%CD%\..\.." -DCMAKE_CONFIGURATION_TYPES:STRING=%CONFIGURATION% -DCMAKE_BUILD_TYPE:STRING=%CONFIGURATION%
@popd
@popd

@echo on
@pause
