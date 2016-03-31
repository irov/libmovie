#	include "movie/movie_instance.h"

#	include <stddef.h>
#	include <malloc.h>

static void * stdlib_movie_alloc( void * _data, uint32_t _size )
{
	(void)_data;

	return malloc( _size );
}

static void stdlib_movie_free( void * _data, void * _ptr )
{
	(void)_data;

	free( _ptr );
}

void make_movie_instance( aeMovieInstance * _instance, movie_alloc_t _alloc, movie_free_t _free, void * _data )
{
	_instance->memory_alloc = _alloc;
	_instance->memory_free = _free;
	_instance->memory_data = _data;

	if( _instance->memory_alloc == NULL )
	{
		_instance->memory_alloc = &stdlib_movie_alloc;
	}

	if( _instance->memory_free == NULL )
	{
		_instance->memory_free = &stdlib_movie_free;
	}
}