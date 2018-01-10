cmake_minimum_required(VERSION 3.9)

project(cc_movie)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall /WX")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4710") #function not inlined 
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4738") #storing 32-bit float result in memory, possible loss of performance
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W4 /WX")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4710") #function not inlined 
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4738") #storing 32-bit float result in memory, possible loss of performance
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

set(cc_movie_dependencies ../../dependencies)

include_directories( ${PROJECT_NAME} ../../include)

INCLUDE_DIRECTORIES( cocos2dx include )
INCLUDE_DIRECTORIES( cocos2dx ${cc_movie_dependencies}/cocos2d-x/cocos )
INCLUDE_DIRECTORIES( cocos2dx ${cc_movie_dependencies}/cocos2d-x/extensions )
INCLUDE_DIRECTORIES( cocos2dx ${cc_movie_dependencies}/cocos2d-x/external )
INCLUDE_DIRECTORIES( cocos2dx ${cc_movie_dependencies}/cocos2d-x/external/glfw3/include/win32 )
INCLUDE_DIRECTORIES( cocos2dx ${cc_movie_dependencies}/cocos2d-x/external/win32-specific/gles/include/OGLES )

ADD_DEFINITIONS(-DCC_STATIC)

if( MSVC )
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
endif()

file(GLOB cc_player_headers src/*.h)
file(GLOB cc_player_sources src/*.cpp)

ADD_LIBRARY( ${PROJECT_NAME} STATIC ${cc_player_headers} ${cc_player_sources})


TARGET_LINK_LIBRARIES(${PROJECT_NAME} movie)
#
