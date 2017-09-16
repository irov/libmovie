@echo off

set "build_dir=build"
set "configuration=Debug"
set "CMAKE_EXE=%CD%\..\..\cmake\bin\cmake.exe"
set "SOLUTION_DIR=solutions\libmovie_msvc15_debug"

@pushd ..
@mkdir %SOLUTION_DIR%
@pushd %SOLUTION_DIR%
%CMAKE_EXE% -G "Visual Studio 15 2017" "%CD%\..\.." -DCMAKE_CONFIGURATION_TYPES:STRING='Debug' -DCMAKE_BUILD_TYPE:STRING='Debug' -DLIBMOVIE_EXAMPLES_BUILD:BOOL=TRUE -DLIBMOVIE_COCOS2DX_BUILD:BOOL=FALSE
@popd

@echo on
@pause
