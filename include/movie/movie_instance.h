#ifndef MOVIE_MOVIE_INSTANCE_H_
#define MOVIE_MOVIE_INSTANCE_H_

#	include "movie_type.h"
#	include "movie_typedef.h"

typedef void * (*ae_movie_alloc_t)(void * _data, size_t _size);
typedef void * (*ae_movie_alloc_n_t)(void * _data, size_t _size, size_t _n);
typedef void( *ae_movie_free_t )(void * _data, const void * _ptr);
typedef void( *ae_movie_free_n_t )(void * _data, const void * _ptr);

typedef int32_t( *ae_movie_strncmp_t )(void * _data, const ae_char_t * _src, const ae_char_t * _dst, size_t _count);

typedef enum
{
	AE_ERROR_INFO,
	AE_ERROR_MEMORY,
	AE_ERROR_STREAM,
	AE_ERROR_WARNING,
	AE_ERROR_INTERNAL,
	AE_ERROR_UNSUPPORT,
	AE_ERROR_CRITICAL
} aeMovieErrorCode;

typedef void( *ae_movie_logger_t )(void * _data, aeMovieErrorCode _code, const ae_char_t * _message, ...);

aeMovieInstance * ae_create_movie_instance(
	ae_movie_alloc_t _alloc,
	ae_movie_alloc_n_t _alloc_n,
	ae_movie_free_t _free,
	ae_movie_free_n_t _free_n,
	ae_movie_strncmp_t _strncmp,
	ae_movie_logger_t _logger,
	void * _data );

void ae_delete_movie_instance( aeMovieInstance * _instance );

#endif