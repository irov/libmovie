#   pragma once

#   include <emscripten.h>

typedef void * em_handle_t;
typedef em_handle_t em_movie_instance_t;
typedef em_handle_t em_movie_data_t;
typedef em_handle_t em_movie_composition_t;

EMSCRIPTEN_KEEPALIVE em_movie_instance_t em_create_movie_instance( const char * _hashkey );
EMSCRIPTEN_KEEPALIVE void em_delete_movie_instance( em_movie_instance_t _instance );

EMSCRIPTEN_KEEPALIVE em_movie_data_t em_create_movie_data( em_movie_instance_t _instance, const char * _path );
EMSCRIPTEN_KEEPALIVE void em_delete_movie_data( em_movie_data_t _movieData );

EMSCRIPTEN_KEEPALIVE em_movie_composition_t em_create_movie_composition( em_movie_data_t _movieData, const char * _name );
EMSCRIPTEN_KEEPALIVE void em_delete_movie_composition( em_movie_composition_t _movieComposition );

EMSCRIPTEN_KEEPALIVE void em_set_movie_composition_loop( em_movie_composition_t _movieComposition );


