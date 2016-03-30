#	include "movie/movie.h"

#	include "memory.h"

void make_instance( aeInstance * _instance, movie_alloc_t _alloc, movie_free_t _free, void * _data )
{
	_instance->memory_alloc = _alloc;
	_instance->memory_free = _free;
	_instance->memory_data = _data;

	if( _instance->memory_alloc == nullptr )
	{
		_instance->memory_alloc = &default_movie_alloc;
	}

	if( _instance->memory_free == nullptr )
	{
		_instance->memory_free = &default_movie_free;
	}
}

aeMovie * create_movie( aeInstance * _instance )
{
	aeMovie * m = NEW( _instance, aeMovie );

	return m;
}

void delete_movie( aeInstance * _instance, aeMovie * _movie )
{
	DELETE( _instance, _movie );
}

aeResult load_movie( aeInstance * _instance, aeMovie * _movie, movie_read_t _read, movie_seek_t _seek, void * _data )
{
	return MOVIE_SUCCESSFUL;
}