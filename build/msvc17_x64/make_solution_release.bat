@echo off

set "CONFIGURATION=Release"
set "SOLUTION_DIR=..\solutions\libmovie_msvc17_x64_%CONFIGURATION%"

@pushd ..
@mkdir %SOLUTION_DIR%
@pushd %SOLUTION_DIR%

CMake -G "Visual Studio 17 2022" -A x64 "%CD%\..\.." -DCMAKE_CONFIGURATION_TYPES:STRING=%CONFIGURATION% -DCMAKE_BUILD_TYPE:STRING=%CONFIGURATION% -DLIBMOVIE_EXAMPLES_BUILD:BOOL=TRUE

@popd
@popd

@echo on
@pause
