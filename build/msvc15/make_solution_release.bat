@echo off

set "CONFIGURATION=Release"
set "SOLUTION_DIR=..\solutions\libmovie_msvc15_release"

@pushd ..
@mkdir %SOLUTION_DIR%
@pushd %SOLUTION_DIR%
CMake -G "Visual Studio 15 2017" "%CD%\..\.." -DCMAKE_CONFIGURATION_TYPES:STRING=%CONFIGURATION% -DCMAKE_BUILD_TYPE:STRING=%CONFIGURATION% -DLIBMOVIE_EXAMPLES_BUILD:BOOL=TRUE
@popd
@popd

@echo on
@pause
