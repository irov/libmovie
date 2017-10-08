#pragma once

#include <emscripten.h>

#include <stdint.h>

typedef void * em_handle_t;
typedef em_handle_t em_player_handle_t;
typedef em_handle_t em_movie_data_handle_t;
typedef em_handle_t em_movie_composition_handle_t;

EMSCRIPTEN_KEEPALIVE em_player_handle_t em_create_player( const char * _hashkey, uint32_t _width, uint32_t _height, uint32_t _ud );
EMSCRIPTEN_KEEPALIVE void em_delete_player( em_player_handle_t _instance );

EMSCRIPTEN_KEEPALIVE em_movie_data_handle_t em_create_movie_data( em_player_handle_t _player, const uint8_t * _data );
EMSCRIPTEN_KEEPALIVE void em_delete_movie_data( em_movie_data_handle_t _movieData );

EMSCRIPTEN_KEEPALIVE em_movie_composition_handle_t em_create_movie_composition( em_player_handle_t _player, em_movie_data_handle_t _movieData, const char * _name );
EMSCRIPTEN_KEEPALIVE void em_delete_movie_composition( em_movie_composition_handle_t _movieComposition );

EMSCRIPTEN_KEEPALIVE void em_set_movie_composition_loop( em_movie_composition_handle_t _movieComposition, unsigned int _loop );

EMSCRIPTEN_KEEPALIVE void em_play_movie_composition( em_movie_composition_handle_t _movieComposition, float _time );

EMSCRIPTEN_KEEPALIVE void em_set_movie_wm( em_player_handle_t _player, float _px, float _py, float _ox, float _oy, float _sx, float _sy, float _angle );
EMSCRIPTEN_KEEPALIVE void em_update_movie_composition( em_player_handle_t _player, em_movie_composition_handle_t _movieComposition, float _time );
EMSCRIPTEN_KEEPALIVE void em_render_movie_composition( em_player_handle_t _player, em_movie_composition_handle_t _movieComposition );

EMSCRIPTEN_KEEPALIVE void em_utils_opengl_create_texture( uint32_t _id, uint32_t _width, uint32_t _height, const uint8_t * _data );


