#	include "movie/movie_instance.h"

//////////////////////////////////////////////////////////////////////////
void make_movie_instance( aeMovieInstance * _instance, movie_alloc_t _alloc, movie_alloc_n_t _alloc_n, movie_free_t _free, movie_free_n_t _free_n, void * _data )
{
	_instance->memory_alloc = _alloc;
	_instance->memory_alloc_n = _alloc_n;
	_instance->memory_free = _free;
	_instance->memory_free_n = _free_n;
	_instance->memory_data = _data;

	float * sprite_uv = _instance->sprite_uv;

	*sprite_uv++ = 0.f;
	*sprite_uv++ = 0.f;
	*sprite_uv++ = 1.f;
	*sprite_uv++ = 0.f;
	*sprite_uv++ = 1.f;
	*sprite_uv++ = 1.f;
	*sprite_uv++ = 0.f;
	*sprite_uv++ = 1.f;

	uint16_t * sprite_indices = _instance->sprite_indices;

	*sprite_indices++ = 0;
	*sprite_indices++ = 3;
	*sprite_indices++ = 1;
	*sprite_indices++ = 1;
	*sprite_indices++ = 3;
	*sprite_indices++ = 2;
}
//////////////////////////////////////////////////////////////////////////