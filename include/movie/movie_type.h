#ifndef MOVIE_TYPE_H_
#define MOVIE_TYPE_H_

#	include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
		
	typedef uint8_t ae_bool_t;
	typedef char * ae_string_t;

	static const uint8_t AE_TRUE = 1;
	static const uint8_t AE_FALSE = 0;
	static void * AE_NULL = 0;

	typedef struct
	{
		uint32_t point_count;
		float * points;

	} aeMoviePolygon;

	typedef struct
	{
		float begin_x;
		float begin_y;
		float end_x;
		float end_y;

	} aeMovieViewport;

	typedef struct
	{
		uint32_t vertex_count;
		uint32_t indices_count;
		float * vertices;
		uint16_t * indices;

	} aeMovieMesh;

	typedef enum
	{
		AE_MOVIE_BLEND_NORMAL = 0,
		AE_MOVIE_BLEND_ADD = 1,
		AE_MOVIE_BLEND_SCREEN = 2,
		AE_MOVIE_BLEND_MULTIPLY = 3,
		__AE_MOVIE_BLEND_END__
	} aeMovieBlendMode;

	typedef enum
	{
		AE_MOVIE_PARAM_LOOP = 0x00000008,
		AE_MOVIE_PARAM_SWITCH = 0x00000010,
		__AE_MOVIE_PARAM_END__
	} aeMovieFlags;

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