#ifndef MOVIE_MOVIE_H_
#define MOVIE_MOVIE_H_

#	include <movie/type.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef void * (*movie_alloc_t)(void * _data, uint32_t _size);
	typedef void (*movie_free_t)(void * _data, void * _ptr);

	typedef struct
	{
		movie_alloc_t memory_alloc;
		movie_free_t memory_free;
		void * memory_data;
	} aeInstance;

	void make_instance( aeInstance * _instance, movie_alloc_t _alloc, movie_free_t _free, void * _data );

	typedef struct
	{
		const char* const name;
		float duration;
	} aeMovie;

	aeMovie * create_movie( aeInstance * _instance );
	void delete_movie( aeInstance * _instance, aeMovie * _movie );

	typedef void( *movie_read_t )(void * _data, void * _buff, uint32_t _size);
	typedef void( *movie_seek_t )(void * _data, uint32_t _size );

	aeResult load_movie( aeInstance * _instance, aeMovie * _movie, movie_read_t _read, movie_seek_t _seek, void * _data );

#ifdef __cplusplus
}
#endif

#endif