#	include "movie/movie_instance.h"

#	include "movie_struct.h"

//////////////////////////////////////////////////////////////////////////
static int32_t __ae_strncmp( void * _data, const ae_char_t * _src, const ae_char_t * _dst, uint32_t _count )
{
	(void)_data;

	if( _count == 0 )
	{
		return 0;
	}

	int32_t cmp = 0;

	while( !(cmp = *(uint8_t *)_src - *(uint8_t *)_dst) && *_dst && _count-- )
	{
		++_src;
		++_dst;
	}

	return cmp;
}
//////////////////////////////////////////////////////////////////////////
static void __ae_movie_logerror( void * _data, aeMovieErrorCode _code, const ae_char_t * _message, ... )
{
	(void)_data;
	(void)_code;
	(void)_message;
	//SILENT
}
//////////////////////////////////////////////////////////////////////////
static void __ae_movie_instance_setup_bezier_warp_uv( aeMovieInstance * instance )
{
	float * bezier_warp_uv = &instance->bezier_warp_uv[0][0];

	uint32_t v = 0;
	for( ; v != AE_MOVIE_BEZIER_WARP_GRID; ++v )
	{
		uint32_t u = 0;
		for( ; u != AE_MOVIE_BEZIER_WARP_GRID; ++u )
		{
			*bezier_warp_uv++ = (float)u * ae_movie_bezier_warp_grid_invf;
			*bezier_warp_uv++ = (float)v * ae_movie_bezier_warp_grid_invf;
		}
	}
}
//////////////////////////////////////////////////////////////////////////
static void __ae_movie_instance_setup_bezier_warp_indices( aeMovieInstance * instance )
{
	uint16_t * bezier_warp_indices = instance->bezier_warp_indices;

	uint16_t v = 0;
	for( ; v != AE_MOVIE_BEZIER_WARP_GRID - 1; ++v )
	{
		uint16_t u = 0;
		for( ; u != AE_MOVIE_BEZIER_WARP_GRID - 1; ++u )
		{
			*bezier_warp_indices++ = u + (v + 0) * AE_MOVIE_BEZIER_WARP_GRID + 0;
			*bezier_warp_indices++ = u + (v + 1) * AE_MOVIE_BEZIER_WARP_GRID + 0;
			*bezier_warp_indices++ = u + (v + 0) * AE_MOVIE_BEZIER_WARP_GRID + 1;
			*bezier_warp_indices++ = u + (v + 0) * AE_MOVIE_BEZIER_WARP_GRID + 1;
			*bezier_warp_indices++ = u + (v + 1) * AE_MOVIE_BEZIER_WARP_GRID + 0;
			*bezier_warp_indices++ = u + (v + 1) * AE_MOVIE_BEZIER_WARP_GRID + 1;
		}
	}
}
//////////////////////////////////////////////////////////////////////////
aeMovieInstance * ae_create_movie_instance( ae_movie_alloc_t _alloc, ae_movie_alloc_n_t _alloc_n, ae_movie_free_t _free, ae_movie_free_n_t _free_n, ae_movie_strncmp_t _strncmp, ae_movie_logerror_t _error, void * _data )
{
	aeMovieInstance * instance = (*_alloc)(_data, sizeof( aeMovieInstance ));

	instance->memory_alloc = _alloc;
	instance->memory_alloc_n = _alloc_n;
	instance->memory_free = _free;
	instance->memory_free_n = _free_n;
	instance->strncmp = _strncmp;
	instance->logerror = _error;
	instance->instance_data = _data;

	if( instance->strncmp == AE_NULL )
	{
		instance->strncmp = &__ae_strncmp;
	}

	if( instance->logerror == AE_NULL )
	{
		instance->logerror = &__ae_movie_logerror;
	}

	float * sprite_uv = &instance->sprite_uv[0][0];

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

	__ae_movie_instance_setup_bezier_warp_uv( instance );
	__ae_movie_instance_setup_bezier_warp_indices( instance );

	return instance;
}
//////////////////////////////////////////////////////////////////////////
void ae_delete_movie_instance( aeMovieInstance * _instance )
{
	(*_instance->memory_free)(_instance->instance_data, _instance);
}
//////////////////////////////////////////////////////////////////////////