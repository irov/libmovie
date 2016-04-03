#	include "movie/movie_instance.h"

//////////////////////////////////////////////////////////////////////////
void make_movie_instance( aeMovieInstance * _instance, movie_alloc_t _alloc, movie_free_t _free, void * _data )
{
	_instance->memory_alloc = _alloc;
	_instance->memory_free = _free;
	_instance->memory_data = _data;
}
//////////////////////////////////////////////////////////////////////////