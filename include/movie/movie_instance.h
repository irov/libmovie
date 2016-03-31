#ifndef MOVIE_MOVIE_INSTANCE_H_
#define MOVIE_MOVIE_INSTANCE_H_

#	include <movie/movie_type.h>

#ifdef __cplusplus
extern "C" {
#endif

	static const uint32_t ae_movie_version = 1;

	typedef void * (*movie_alloc_t)(void * _data, uint32_t _size);
	typedef void( *movie_free_t )(void * _data, void * _ptr);

	typedef struct aeMovieInstance
	{
		movie_alloc_t memory_alloc;
		movie_free_t memory_free;
		void * memory_data;
	} aeMovieInstance;

	void make_movie_instance( aeMovieInstance * _instance, movie_alloc_t _alloc, movie_free_t _free, void * _data );


#ifdef __cplusplus
}
#endif

#endif