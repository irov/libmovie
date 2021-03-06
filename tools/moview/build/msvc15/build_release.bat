@echo off

set "PROJECT=moview"
set "CONFIGURATION=Release"
set "SOLUTION_TYPE=MSVC"
set "SOLUTION_VERSION=15"
set "SOLUTION_DIR=..\solutions\%PROJECT%_%SOLUTION_TYPE%%SOLUTION_VERSION%_%CONFIGURATION%"

@pushd ..
@mkdir %SOLUTION_DIR%
@pushd %SOLUTION_DIR%
CMake -G "Visual Studio 15 2017" "%CD%\..\.." -DCMAKE_CONFIGURATION_TYPES:STRING=%CONFIGURATION% -DCMAKE_BUILD_TYPE:STRING=%CONFIGURATION% -D%SOLUTION_TYPE%:BOOLEAN=ON
CMake --build . --config %CONFIGURATION%
@popd
@popd

@echo on
@pause
