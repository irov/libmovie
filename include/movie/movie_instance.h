#ifndef MOVIE_MOVIE_INSTANCE_H_
#define MOVIE_MOVIE_INSTANCE_H_

#	include <movie/movie_type.h>

#ifdef __cplusplus
extern "C" {
#endif

	static const uint32_t ae_movie_version = 1;

	typedef void * (*ae_movie_alloc_t)(void * _data, uint32_t _size);
	typedef void * (*ae_movie_alloc_n_t)(void * _data, uint32_t _size, uint32_t _n);
	typedef void( *ae_movie_free_t )(void * _data, const void * _ptr);
	typedef void( *ae_movie_free_n_t )(void * _data, const void * _ptr);

	typedef struct aeMovieInstance
	{
		ae_movie_alloc_t memory_alloc;
		ae_movie_alloc_n_t memory_alloc_n;
		ae_movie_free_t memory_free;
		ae_movie_free_n_t memory_free_n;
		void * memory_data;

		float sprite_uv[8];
		uint16_t sprite_indices[6];
	} aeMovieInstance;

	void ae_make_movie_instance( aeMovieInstance * _instance, ae_movie_alloc_t _alloc, ae_movie_alloc_n_t _alloc_n, ae_movie_free_t _free, ae_movie_free_n_t _free_n, void * _data );

#ifdef __cplusplus
}
#endif

#endif