cmake_minimum_required(VERSION 3.10)

PROJECT(movie C)

OPTION(LIBMOVIE_EXTERNAL_BUILD  "LIBMOVIE_EXTERNAL_BUILD" OFF)
OPTION(LIBMOVIE_EXAMPLES_BUILD  "LIBMOVIE_EXAMPLES_BUILD" ON)
OPTION(LIBMOVIE_COVERAGE  "LIBMOVIE_COVERAGE" OFF)
OPTION(LIBMOVIE_INSTALL  "LIBMOVIE_INSTALL" OFF)
OPTION(LIBMOVIE_TEST  "LIBMOVIE_TEST" OFF)
OPTION(LIBMOVIE_MEMORY_DEBUG "LIBMOVIE_MEMORY_DEBUG" OFF)

IF( NOT LIBMOVIE_EXTERNAL_BUILD )
    if(${CMAKE_C_COMPILER_ID} STREQUAL Clang)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
    elseif(${CMAKE_C_COMPILER_ID} STREQUAL AppleClang)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
    elseif(${CMAKE_C_COMPILER_ID} STREQUAL GNU)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
    elseif(${CMAKE_C_COMPILER_ID} STREQUAL Intel)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
    elseif(${CMAKE_C_COMPILER_ID} STREQUAL MSVC)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Wall /WX")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd5045")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4668")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4710")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4738")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4820")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4711")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4061")
    endif()
ENDIF()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)

IF(LIBMOVIE_COVERAGE AND CMAKE_COMPILER_IS_GNUCXX)
    include(CodeCoverage)
    setup_target_for_coverage(${PROJECT_NAME}_coverage tests coverage)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
ENDIF()

MACRO( ADD_FILTER group_name )
    SOURCE_GROUP( ${group_name} FILES ${ARGN} )
    SET( SRC_FILES ${SRC_FILES} ${ARGN} )
ENDMACRO()

SET( INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include )
SET( SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src )

ADD_FILTER(
public
    ${INCLUDE_DIR}/movie/movie.h
    ${INCLUDE_DIR}/movie/movie.hpp
    ${INCLUDE_DIR}/movie/movie_config.h
    ${INCLUDE_DIR}/movie/movie_data.h
    ${INCLUDE_DIR}/movie/movie_instance.h
    ${INCLUDE_DIR}/movie/movie_composition.h
    ${INCLUDE_DIR}/movie/movie_render.h
    ${INCLUDE_DIR}/movie/movie_providers.h
    ${INCLUDE_DIR}/movie/movie_resource.h
    ${INCLUDE_DIR}/movie/movie_skeleton.h
    ${INCLUDE_DIR}/movie/movie_type.h
    ${INCLUDE_DIR}/movie/movie_typedef.h
    ${INCLUDE_DIR}/movie/movie_version.h
)

ADD_FILTER(
source
    ${SOURCE_DIR}/movie_detail.h
    ${SOURCE_DIR}/movie_bezier.c
    ${SOURCE_DIR}/movie_bezier.h
    ${SOURCE_DIR}/movie_data.c
    ${SOURCE_DIR}/movie_data_validate.c
    ${SOURCE_DIR}/movie_data_validate.h
    ${SOURCE_DIR}/movie_debug.h
    ${SOURCE_DIR}/movie_instance.c
    ${SOURCE_DIR}/movie_math.c
    ${SOURCE_DIR}/movie_math.h
    ${SOURCE_DIR}/movie_utils.h
    ${SOURCE_DIR}/movie_memory.h
    ${SOURCE_DIR}/movie_composition.c
    ${SOURCE_DIR}/movie_providers.c
    ${SOURCE_DIR}/movie_skeleton.c
    ${SOURCE_DIR}/movie_stream.c
    ${SOURCE_DIR}/movie_stream.h
    ${SOURCE_DIR}/movie_struct.h
    ${SOURCE_DIR}/movie_transformation.c
    ${SOURCE_DIR}/movie_transformation.h
    ${SOURCE_DIR}/movie_motion.c
    ${SOURCE_DIR}/movie_motion.h
    ${SOURCE_DIR}/movie_motion_format.h
    ${INCLUDE_DIR}/movie/movie_motion.h
    ${SOURCE_DIR}/movie_property.c
    ${SOURCE_DIR}/movie_property.h
)

INCLUDE_DIRECTORIES( ${PROJECT_NAME} ${INCLUDE_DIR} )

if(LIBMOVIE_MEMORY_DEBUG)
    ADD_DEFINITIONS(-DAE_MOVIE_MEMORY_DEBUG)
endif()

ADD_LIBRARY( ${PROJECT_NAME} STATIC ${SRC_FILES} )

if(LIBMOVIE_INSTALL)
    install(DIRECTORY include
            DESTINATION .
            FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")

    install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib)
endif()

if(LIBMOVIE_EXAMPLES_BUILD)
    add_subdirectory(examples)
endif()

if(LIBMOVIE_TEST)
    include(CTest)
    enable_testing()

    add_subdirectory(tests)
endif()
OPTION(LIBMOVIE_TOOLS_BUILD "LIBMOVIE_TOOLS_BUILD" OFF)

IF(LIBMOVIE_TOOLS_BUILD)
    FIND_PATH(JANSSON_INCLUDE_DIR jansson.h REQUIRED)
    FIND_LIBRARY(JANSSON_LIBRARY NAMES jansson REQUIRED)
    FIND_PATH(LZ4_INCLUDE_DIR lz4.h REQUIRED)
    FIND_LIBRARY(LZ4_LIBRARY NAMES lz4 REQUIRED)

    ADD_LIBRARY(moviec_core STATIC
        tools/moviec/moviec.h
        tools/moviec/moviec_internal.h
        tools/moviec/moviec_cli.c
        tools/moviec/moviec_collect.c
        tools/moviec/moviec_compile.c
        tools/moviec/moviec_context.c
        tools/moviec/moviec_expression.c
        tools/moviec/moviec_geometry.c
        tools/moviec/moviec_package.c
        tools/moviec/moviec_scaffold.c
    )

    TARGET_COMPILE_FEATURES(moviec_core PUBLIC c_std_11)
    TARGET_INCLUDE_DIRECTORIES(moviec_core
        PUBLIC tools/moviec ${JANSSON_INCLUDE_DIR}
        PRIVATE ${LZ4_INCLUDE_DIR} ${SOURCE_DIR}
    )
    TARGET_LINK_LIBRARIES(moviec_core PUBLIC movie ${JANSSON_LIBRARY} ${LZ4_LIBRARY})

    ADD_EXECUTABLE(moviec tools/moviec/moviec.c)
    TARGET_LINK_LIBRARIES(moviec PRIVATE moviec_core)

    IF(MSVC)
        TARGET_COMPILE_OPTIONS(moviec_core PRIVATE /W4 /WX)
    ELSE()
        TARGET_COMPILE_OPTIONS(moviec_core PRIVATE -Wall -Wextra -Werror)
    ENDIF()
ENDIF()

IF(LIBMOVIE_TOOLS_BUILD AND LIBMOVIE_TEST)
    ADD_EXECUTABLE(test_motion tests/test_motion.c)
    TARGET_LINK_LIBRARIES(test_motion PRIVATE movie)

    ADD_TEST(NAME motion_compile COMMAND moviec build ${CMAKE_CURRENT_SOURCE_DIR}/examples/authoring/kinematics.movie -o ${CMAKE_CURRENT_BINARY_DIR}/kinematics.aem)
    SET_TESTS_PROPERTIES(motion_compile PROPERTIES FIXTURES_SETUP motion_document)

    ADD_TEST(NAME motion_runtime COMMAND test_motion ${CMAKE_CURRENT_BINARY_DIR}/kinematics.aem)
    SET_TESTS_PROPERTIES(motion_runtime PROPERTIES FIXTURES_REQUIRED motion_document)

    ADD_EXECUTABLE(test_moviec tests/test_moviec.c)
    TARGET_LINK_LIBRARIES(test_moviec PRIVATE moviec_core)

    FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/moviec_tests)
    ADD_TEST(NAME motion_authoring COMMAND test_moviec ${CMAKE_CURRENT_BINARY_DIR}/moviec_tests)
ENDIF()
