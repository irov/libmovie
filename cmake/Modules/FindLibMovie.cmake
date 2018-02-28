set( LIBMOVIE_FOUND false )

find_path(
        LIBMOVIE_INCLUDE_DIR
        movie
        PATH_SUFFIXES
        include
        PATHS
        /usr
        /usr/local
        ${LIBMOVIE_ROOT}
        $ENV{LIBMOVIE_ROOT}
)

find_library(
        LIBMOVIE_LIBRARY
        libmovie
        PATH_SUFFIXES
		build_temp
        lib
        lib64
        PATHS
        /usr
        /usr/local
        ${LIBMOVIEDIR}
        ${LIBMOVIE_ROOT}
        $ENV{LIBMOVIE_ROOT}
        $ENV{LIBMOVIEDIR}
)

find_library(
        PATH_SUFFIXES
		build_temp
        lib
        lib64
        PATHS
        /usr
        /usr/local
        ${LIBMOVIEDIR}
        ${LIBMOVIE_ROOT}
        $ENV{LIBMOVIE_ROOT}
        $ENV{LIBMOVIEDIR}
)

if(NOT LIBMOVIE_INCLUDE_DIR OR NOT LIBMOVIE_LIBRARY )
    message( FATAL_ERROR "LIBMOVIE not found. Set LIBMOVIE_ROOT to the installation root directory (containing include/ and lib/)" )
else()
    message( STATUS "LIBMOVIE found: ${LIBMOVIE_INCLUDE_DIR}" )
    message( STATUS "LIBMOVIE LIB PATH: ${LIBMOVIE_LIBRARY}")
    set(LIBMOVIE_FOUND TRUE)
endif()