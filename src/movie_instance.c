#	include "movie/movie_instance.h"

//////////////////////////////////////////////////////////////////////////
void make_movie_instance( aeMovieInstance * _instance, movie_alloc_t _alloc, movie_alloc_n_t _alloc_n, movie_free_t _free, movie_free_n_t _free_n, void * _data )
{
	_instance->memory_alloc = _alloc;
	_instance->memory_alloc_n = _alloc_n;
	_instance->memory_free = _free;
	_instance->memory_free_n = _free_n;
	_instance->memory_data = _data;
}
//////////////////////////////////////////////////////////////////////////