cmake_minimum_required(VERSION 3.9)
project(em_movieplayer)
include_directories( ${PROJECT_NAME} ${EMSCRIPTEN_DIR}/system/include)
include_directories( ${PROJECT_NAME} ../../include)
set(CMAKE_C_FLAGS  "-s EXPORT_NAME=\"'AEMovie'\" -s MODULARIZE=1 --memory-init-file 0 \
-O2 \
-s FORCE_FILESYSTEM=1 \
-s 'EXTRA_EXPORTED_RUNTIME_METHODS=[\"GL\", \"FS\"]' \
-s USE_SDL_IMAGE=2 \
-s SDL2_IMAGE_FORMATS='[\"png\"]' \
--embed-file ui.aem \
--embed-file images/1024x748.png \
")
file(GLOB em_player_sources *.c)
file(GLOB ae_movie_sources ../../src/*.c)
add_executable( ${PROJECT_NAME} ${em_player_sources} ${ae_movie_sources})