#ifndef MOVIE_TYPE_H_
#define MOVIE_TYPE_H_

#	include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef int8_t result_t;
	typedef char * ae_string_t;

	typedef enum
	{
		AE_MOVIE_SUCCESSFUL = 0,
		AE_MOVIE_FAILED = -1
	} aeMovieResult;

	typedef void( *movie_stream_read_t )(void * _data, void * _buff, uint32_t _size);

	typedef struct
	{
		movie_stream_read_t read;
		void * data;
	} aeMovieStream;
	
#ifdef __cplusplus
}
#endif

#endif