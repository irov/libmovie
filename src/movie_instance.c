#	include "movie/movie_instance.h"

#	include "movie_struct.h"

//////////////////////////////////////////////////////////////////////////
aeMovieInstance * ae_create_movie_instance( ae_movie_alloc_t _alloc, ae_movie_alloc_n_t _alloc_n, ae_movie_free_t _free, ae_movie_free_n_t _free_n, void * _data )
{
	aeMovieInstance * instance = (*_alloc)(_data, sizeof( aeMovieInstance ));

	instance->memory_alloc = _alloc;
	instance->memory_alloc_n = _alloc_n;
	instance->memory_free = _free;
	instance->memory_free_n = _free_n;
	instance->memory_data = _data;

	float * sprite_uv = instance->sprite_uv;

	*sprite_uv++ = 0.f;
	*sprite_uv++ = 0.f;
	*sprite_uv++ = 1.f;
	*sprite_uv++ = 0.f;
	*sprite_uv++ = 1.f;
	*sprite_uv++ = 1.f;
	*sprite_uv++ = 0.f;
	*sprite_uv++ = 1.f;

	uint16_t * sprite_indices = instance->sprite_indices;

	*sprite_indices++ = 0;
	*sprite_indices++ = 3;
	*sprite_indices++ = 1;
	*sprite_indices++ = 1;
	*sprite_indices++ = 3;
	*sprite_indices++ = 2;

	return instance;
}
//////////////////////////////////////////////////////////////////////////
void ae_delete_movie_instance( aeMovieInstance * _instance )
{
	(*_instance->memory_free)(_instance->memory_data, _instance);
}
//////////////////////////////////////////////////////////////////////////