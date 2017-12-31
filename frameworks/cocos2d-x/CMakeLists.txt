cmake_minimum_required(VERSION 3.9)

project(cc_movie)

set(cc_movie_dependencies ../../dependencies)

include_directories( ${PROJECT_NAME} ../../include)

INCLUDE_DIRECTORIES( cocos2dx include )
INCLUDE_DIRECTORIES( cocos2dx ${cc_movie_dependencies}/cocos2d-x )
INCLUDE_DIRECTORIES( cocos2dx ${cc_movie_dependencies}/cocos2d-x/cocos )
INCLUDE_DIRECTORIES( cocos2dx ${cc_movie_dependencies}/cocos2d-x/external )
INCLUDE_DIRECTORIES( cocos2dx ${cc_movie_dependencies}/cocos2d-x/external/glfw3/include/win32 )
INCLUDE_DIRECTORIES( cocos2dx ${cc_movie_dependencies}/cocos2d-x/external/win32-specific/gles/include/OGLES )

ADD_DEFINITIONS(-DCC_STATIC)
ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251 /wd4273")

file(GLOB cc_player_headers src/*.h)
file(GLOB cc_player_sources src/*.cpp)

ADD_LIBRARY( ${PROJECT_NAME} STATIC ${cc_player_headers} ${cc_player_sources})


TARGET_LINK_LIBRARIES(${PROJECT_NAME} movie)
#
