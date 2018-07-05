cmake_minimum_required(VERSION 3.0)

set(LIBMOVIE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(DEPENDENCIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies)

include(cmake/glad_download.cmake)
include(cmake/glfw_download.cmake)
include(cmake/imgui_download.cmake)
include(cmake/nativefiledialog_download.cmake)
include(cmake/stb_image_download.cmake)

project( moview )

set(LIBMOVIE_EXTERNAL_BUILD ON CACHE BOOL "LIBMOVIE_EXTERNAL_BUILD" FORCE)
set(LIBMOVIE_EXAMPLES_BUILD OFF CACHE BOOL "LIBMOVIE_EXAMPLES_BUILD" FORCE)
set(LIBMOVIE_INSTALL ON CACHE BOOL "LIBMOVIE_INSTALL" FORCE)
add_subdirectory(${LIBMOVIE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/movie)

ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)

set(CMAKE_CXX14_EXTENSION_COMPILE_OPTION -std=c++14)
set(CMAKE_CXX_STANDARD 14)

set(MOVIEW_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(MOVIEW_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB files_headers ${MOVIEW_SOURCE_DIR}/*.h)
file(GLOB files_sources ${MOVIEW_SOURCE_DIR}/*.cpp)
		
include_directories(
	${MOVIEW_INCLUDE_DIR}
	${GLFW_INCLUDE_DIR}
	${IMGUI_INCLUDE_DIR}
	${GLAD_INCLUDE_DIR}
	${NATIVEFILEDIALOG_INCLUDE_DIR}
	${LIBMOVIE_DIR}/include
	${DEPENDENCIES_DIR}/stb
)

add_executable(${PROJECT_NAME}	
	${files_headers} 
	${files_sources}
)

target_link_libraries(${PROJECT_NAME}
	movie
	glad
	glfw
	imgui
	nativefiledialog
)

add_dependencies(${PROJECT_NAME}
	glad_download
    glfw_download
	imgui_download
	nativefiledialog_download
)