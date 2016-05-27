#ifndef MOVIE_MOVIE_INSTANCE_H_
#define MOVIE_MOVIE_INSTANCE_H_

#	include <movie/movie_type.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef void * (*ae_movie_alloc_t)(void * _data, uint32_t _size);
	typedef void * (*ae_movie_alloc_n_t)(void * _data, uint32_t _size, uint32_t _n);
	typedef void( *ae_movie_free_t )(void * _data, const void * _ptr);
	typedef void( *ae_movie_free_n_t )(void * _data, const void * _ptr);

	typedef struct aeMovieInstance aeMovieInstance;

	aeMovieInstance * ae_create_movie_instance( ae_movie_alloc_t _alloc, ae_movie_alloc_n_t _alloc_n, ae_movie_free_t _free, ae_movie_free_n_t _free_n, void * _data );
	void ae_delete_movie_instance( aeMovieInstance * _instance );

#ifdef __cplusplus
}
#endif

#endif