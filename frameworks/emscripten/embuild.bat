mkdir build
pushd build

del "../bin/em_movie.js"

cmake.exe -G "Unix Makefiles" .. -DEMSCRIPTEN_DIR=%EMSCRIPTEN% -DCMAKE_TOOLCHAIN_FILE=%EMSCRIPTEN%\cmake\Modules\Platform\Emscripten.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_MAKE_PROGRAM=mingw32-make
mingw32-make

@popd