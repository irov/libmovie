cmake_minimum_required(VERSION 3.0)

PROJECT(movie)

OPTION(LIBMOVIE_EXTERNAL_BUILD  "LIBMOVIE_EXTERNAL_BUILD"  OFF)
OPTION(LIBMOVIE_EXAMPLES_BUILD  "LIBMOVIE_EXAMPLES_BUILD"  OFF)

IF( NOT LIBMOVIE_EXTERNAL_BUILD )
    SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build_temp/${ENGINE_LIB_DIR} )
    SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/libmovie_win32 )
    SET( CMAKE_TEMP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/CMakeTemp/libmovie_win32/${CMAKE_GENERATOR}/${CMAKE_BUILD_TYPE} )
ENDIF()

MACRO( ADD_FILTER group_name )
	SOURCE_GROUP( ${group_name} FILES ${ARGN} )
	SET( SRC_FILES ${SRC_FILES} ${ARGN} )
ENDMACRO()

SET( PUBLIC_DIR include/movie )
SET( SOURCE_DIR src )

ADD_FILTER(
public
	${PUBLIC_DIR}/movie.h
    ${PUBLIC_DIR}/movie.hpp
    ${PUBLIC_DIR}/movie_data.h
    ${PUBLIC_DIR}/movie_instance.h
    ${PUBLIC_DIR}/movie_resource.h
    ${PUBLIC_DIR}/movie_skeleton.h
    ${PUBLIC_DIR}/movie_type.h
    ${PUBLIC_DIR}/movie_typedef.h
)

ADD_FILTER(
source
	${SOURCE_DIR}/movie_data.c
    ${SOURCE_DIR}/movie_instance.c
    ${SOURCE_DIR}/movie_math.c
    ${SOURCE_DIR}/movie_math.h
    ${SOURCE_DIR}/movie_memory.h
    ${SOURCE_DIR}/movie_node.c
    ${SOURCE_DIR}/movie_skeleton.c
    ${SOURCE_DIR}/movie_stream.h
    ${SOURCE_DIR}/movie_struct.h
    ${SOURCE_DIR}/movie_transformation.c
    ${SOURCE_DIR}/movie_transformation.h
)

INCLUDE_DIRECTORIES( ${PROJECT_NAME} include ) 

ADD_LIBRARY( ${PROJECT_NAME} STATIC ${SRC_FILES} )

if( LIBMOVIE_EXAMPLES_BUILD )
    set(
    sandbox_SOURCES
        examples/sandbox/sandbox.c
    )

    INCLUDE_DIRECTORIES( sandbox ../include )

    ADD_EXECUTABLE( sandbox ${sandbox_SOURCES} )

    TARGET_LINK_LIBRARIES( sandbox movie )
    
    set(
    test1_SOURCES
        examples/test/common.c
        examples/test/common.h
        examples/test/test1.c
    )

    INCLUDE_DIRECTORIES( test1 ../include )

    ADD_EXECUTABLE( test1 ${test1_SOURCES} )

    TARGET_LINK_LIBRARIES( test1 movie )    
    
    set(
    test2_SOURCES
        examples/test/common.c
        examples/test/common.h
        examples/test/test2.c
    )

    INCLUDE_DIRECTORIES( test2 ../include )

    ADD_EXECUTABLE( test2 ${test2_SOURCES} )

    TARGET_LINK_LIBRARIES( test2 movie )
    
    set(
    test3_SOURCES
        examples/test/common.c
        examples/test/common.h
        examples/test/test3.c
    )

    INCLUDE_DIRECTORIES( test3 ../include )

    ADD_EXECUTABLE( test3 ${test3_SOURCES} )

    TARGET_LINK_LIBRARIES( test3 movie )    
endif()