#ifndef MOVIE_TYPE_H_
#define MOVIE_TYPE_H_

#	include <stdint.h>
#	include <stddef.h>

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

typedef uint8_t ae_color_t;

typedef struct
{
	uint32_t point_count;
	const ae_vector2_t * points;

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
	const ae_vector2_t * positions;
	const ae_vector2_t * uvs;
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
#	define AE_MOVIE_MAX_VERTICES 1024
#	endif

typedef struct
{
	ae_vector2_t corners[4];
	ae_vector2_t beziers[8];

} aeMovieBezierWarp;

typedef enum
{
	AE_MOVIE_BLEND_ADD = 1,
	AE_MOVIE_BLEND_ALPHA_ADD,
	AE_MOVIE_BLEND_CLASSIC_COLOR_BURN,
	AE_MOVIE_BLEND_CLASSIC_COLOR_DODGE,
	AE_MOVIE_BLEND_CLASSIC_DIFFERENCE,
	AE_MOVIE_BLEND_COLOR,
	AE_MOVIE_BLEND_COLOR_BURN,
	AE_MOVIE_BLEND_COLOR_DODGE,
	AE_MOVIE_BLEND_DANCING_DISSOLVE,
	AE_MOVIE_BLEND_DARKEN,
	AE_MOVIE_BLEND_DARKER_COLOR,
	AE_MOVIE_BLEND_DIFFERENCE,
	AE_MOVIE_BLEND_DISSOLVE,
	AE_MOVIE_BLEND_EXCLUSION,
	AE_MOVIE_BLEND_HARD_LIGHT,
	AE_MOVIE_BLEND_HARD_MIX,
	AE_MOVIE_BLEND_HUE,
	AE_MOVIE_BLEND_LIGHTEN,
	AE_MOVIE_BLEND_LIGHTER_COLOR,
	AE_MOVIE_BLEND_LINEAR_BURN,
	AE_MOVIE_BLEND_LINEAR_DODGE,
	AE_MOVIE_BLEND_LINEAR_LIGHT,
	AE_MOVIE_BLEND_LUMINESCENT_PREMUL,
	AE_MOVIE_BLEND_LUMINOSITY,
	AE_MOVIE_BLEND_MULTIPLY,
	AE_MOVIE_BLEND_NORMAL,
	AE_MOVIE_BLEND_OVERLAY,
	AE_MOVIE_BLEND_PIN_LIGHT,
	AE_MOVIE_BLEND_SATURATION,
	AE_MOVIE_BLEND_SCREEN,
	AE_MOVIE_BLEND_SILHOUETE_ALPHA,
	AE_MOVIE_BLEND_SILHOUETTE_LUMA,
	AE_MOVIE_BLEND_SOFT_LIGHT,
	AE_MOVIE_BLEND_STENCIL_ALPHA,
	AE_MOVIE_BLEND_STENCIL_LUMA,
	AE_MOVIE_BLEND_VIVID_LIGHT,
	__AE_MOVIE_BLEND_END__
} aeMovieBlendMode;

typedef enum
{
	AE_MOVIE_SUCCESSFUL = 0,
	AE_MOVIE_FAILED = -1,
	AE_MOVIE_INVALID_MAGIC = -2,
	AE_MOVIE_INVALID_VERSION = -3,
} aeMovieResult;

typedef size_t( *ae_movie_stream_memory_read_t )(void * _data, void * _buff, size_t _size);
typedef void( *ae_movie_stream_memory_copy_t )(void * _data, const void * _src, void * _dst, size_t _size);

#	ifndef AE_MOVIE_STREAM_NO_CACHE
#	define AE_MOVIE_STREAM_CACHE
#	endif

#	ifndef AE_MOVIE_STREAM_BUFFER_SIZE
#	define AE_MOVIE_STREAM_CACHE_BUFFER_SIZE 32768
#	endif

typedef struct
{
	ae_movie_stream_memory_read_t memory_read;
	void * data;

#	ifdef AE_MOVIE_STREAM_CACHE
	ae_movie_stream_memory_copy_t memory_copy;

	size_t carriage;
	size_t capacity;
	size_t reading;

	uint8_t buff[AE_MOVIE_STREAM_CACHE_BUFFER_SIZE];
#	endif
} aeMovieStream;

#endif