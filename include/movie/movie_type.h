#ifndef MOVIE_TYPE_H_
#define MOVIE_TYPE_H_

#	include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
		
	typedef uint8_t ae_bool_t;
	typedef char ae_char_t;
	typedef ae_char_t * ae_string_t;

	typedef float ae_vector2_t[2];
	typedef float ae_vector3_t[3];
	typedef float ae_vector4_t[4];
	typedef float ae_quaternion_t[4];
	typedef float ae_matrix4_t[16];
	
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
		float * positions;
		float * uvs;
		uint16_t * indices;

	} aeMovieMesh;

#	ifndef AE_MOVIE_MAX_LAYER_NAME
#	define AE_MOVIE_MAX_LAYER_NAME 128
#	endif

#	ifndef AE_MOVIE_BEZIER_WARP_GRID
#	define AE_MOVIE_BEZIER_WARP_GRID 9
#	endif

	static const float ae_movie_bezier_warp_grid_invf = (1.f / (float)(AE_MOVIE_BEZIER_WARP_GRID - 1));

#	define AE_MOVIE_BEZIER_WARP_GRID_VERTEX_COUNT (AE_MOVIE_BEZIER_WARP_GRID * AE_MOVIE_BEZIER_WARP_GRID)
#	define AE_MOVIE_BEZIER_WARP_GRID_INDICES_COUNT ((AE_MOVIE_BEZIER_WARP_GRID - 1) * (AE_MOVIE_BEZIER_WARP_GRID - 1) * 6)

#	ifndef AE_MOVIE_MAX_VERTICES
#	define AE_MOVIE_MAX_VERTICES (AE_MOVIE_BEZIER_WARP_GRID * AE_MOVIE_BEZIER_WARP_GRID)
#	endif

	typedef struct
	{
		ae_vector2_t corners[4];
		ae_vector2_t beziers[8];

		//ae_vector2_t corner_left_top;
		//ae_vector2_t corner_right_top;
		//ae_vector2_t corner_right_bottom;
		//ae_vector2_t corner_left_bottom;

		//ae_vector2_t bezier_left_top_down;
		//ae_vector2_t bezier_left_top_right;

		//ae_vector2_t bezier_right_top_left;
		//ae_vector2_t bezier_right_top_down;

		//ae_vector2_t bezier_right_bottom_up;
		//ae_vector2_t bezier_right_bottom_left;

		//ae_vector2_t bezier_left_bottom_right;
		//ae_vector2_t bezier_left_bottom_up;

	} aeMovieBezierWarp;

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
		AE_MOVIE_SUCCESSFUL = 0,
		AE_MOVIE_FAILED = -1
	} aeMovieResult;

	typedef void( *ae_movie_stream_read_t )(void * _data, void * _buff, uint32_t _size);

	typedef struct
	{
		ae_movie_stream_read_t read;
		void * data;
	} aeMovieStream;
	
#ifdef __cplusplus
}
#endif

#endif