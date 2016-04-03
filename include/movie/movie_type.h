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

	static const uint16_t AE_MOVIE_IMMUTABLE_ANCHOR_POINT_X = 0x0001;
	static const uint16_t AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Y = 0x0002;
	static const uint16_t AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Z = 0x0004;
	static const uint16_t AE_MOVIE_IMMUTABLE_POSITION_X = 0x0008;
	static const uint16_t AE_MOVIE_IMMUTABLE_POSITION_Y = 0x0010;
	static const uint16_t AE_MOVIE_IMMUTABLE_POSITION_Z = 0x0020;
	static const uint16_t AE_MOVIE_IMMUTABLE_ROTATION_X = 0x0040;
	static const uint16_t AE_MOVIE_IMMUTABLE_ROTATION_Y = 0x0080;
	static const uint16_t AE_MOVIE_IMMUTABLE_ROTATION_Z = 0x0100;
	static const uint16_t AE_MOVIE_IMMUTABLE_ROTATION_W = 0x0200;
	static const uint16_t AE_MOVIE_IMMUTABLE_SCALE_X = 0x0400;
	static const uint16_t AE_MOVIE_IMMUTABLE_SCALE_Y = 0x0800;
	static const uint16_t AE_MOVIE_IMMUTABLE_SCALE_Z = 0x1000;
	static const uint16_t AE_MOVIE_IMMUTABLE_OPACITY = 0x2000;
	static const uint16_t AE_MOVIE_IMMUTABLE_VOLUME = 0x4000;

	typedef struct
	{
		uint16_t immutable_mask;
		
		float immuttable_anchor_point_x;
		float immuttable_anchor_point_y;
		float immuttable_anchor_point_z;
		float immuttable_position_x;
		float immuttable_position_y;
		float immuttable_position_z;
		float immuttable_rotation_x;
		float immuttable_rotation_y;
		float immuttable_rotation_z;
		float immuttable_rotation_w;
		float immuttable_scale_x;
		float immuttable_scale_y;
		float immuttable_scale_z;
		float immuttable_opacity;
		float immuttable_volume;

		float * anchor_point_x;
		float * anchor_point_y;
		float * anchor_point_z;
		float * position_x;
		float * position_y;
		float * position_z;
		float * rotation_x;
		float * rotation_y;
		float * rotation_z;
		float * rotation_w;
		float * scale_x;
		float * scale_y;
		float * scale_z;
		float * opacity;
		float * volume;
	} aeMovieLayerProperty;
		
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
		AE_MOVIE_PARAM_TEXT_HORIZONTAL_CENTER = 0x00000001,
		AE_MOVIE_PARAM_TEXT_VERTICAL_CENTER = 0x00000002,
		AE_MOVIE_PARAM_PARTICLE_TRANSLATE = 0x00000004,
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