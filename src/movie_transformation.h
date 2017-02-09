#	ifndef MOVIE_TRANSFORMATION_H_
#	define MOVIE_TRANSFORMATION_H_

#	include "movie/movie_instance.h"
#	include "movie/movie_type.h"

typedef enum aeMoviePropertyImmutableEnum
{	
	AE_MOVIE_IMMUTABLE_ANCHOR_POINT_X = 0x00000001,
	AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Y = 0x00000002,
	AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Z = 0x00000004,
	AE_MOVIE_IMMUTABLE_POSITION_X = 0x00000010,
	AE_MOVIE_IMMUTABLE_POSITION_Y = 0x00000020,
	AE_MOVIE_IMMUTABLE_POSITION_Z = 0x00000040,
	AE_MOVIE_IMMUTABLE_SCALE_X = 0x00000100,
	AE_MOVIE_IMMUTABLE_SCALE_Y = 0x00000200,
	AE_MOVIE_IMMUTABLE_SCALE_Z = 0x00000400,
	AE_MOVIE_IMMUTABLE_QUATERNION_X = 0x00001000,
	AE_MOVIE_IMMUTABLE_QUATERNION_Y = 0x00002000,
	AE_MOVIE_IMMUTABLE_QUATERNION_Z = 0x00004000,
	AE_MOVIE_IMMUTABLE_QUATERNION_W = 0x00008000,
	AE_MOVIE_IMMUTABLE_OPACITY = 0x00010000,
	__AE_MOVIE_IMMUTABLE_NONE__ = 0,
	__AE_MOVIE_IMMUTABLE_TWO_D_ALL__ = 0
	| AE_MOVIE_IMMUTABLE_ANCHOR_POINT_X | AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Y
	| AE_MOVIE_IMMUTABLE_POSITION_X | AE_MOVIE_IMMUTABLE_POSITION_Y
	| AE_MOVIE_IMMUTABLE_SCALE_X | AE_MOVIE_IMMUTABLE_SCALE_Y
	| AE_MOVIE_IMMUTABLE_QUATERNION_Z | AE_MOVIE_IMMUTABLE_QUATERNION_W,
	__AE_MOVIE_IMMUTABLE_THREE_D_ALL__ = 0
	| AE_MOVIE_IMMUTABLE_ANCHOR_POINT_X | AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Y | AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Z
	| AE_MOVIE_IMMUTABLE_POSITION_X | AE_MOVIE_IMMUTABLE_POSITION_Y | AE_MOVIE_IMMUTABLE_POSITION_Z
	| AE_MOVIE_IMMUTABLE_SCALE_X | AE_MOVIE_IMMUTABLE_SCALE_Y | AE_MOVIE_IMMUTABLE_SCALE_Z
	| AE_MOVIE_IMMUTABLE_QUATERNION_X | AE_MOVIE_IMMUTABLE_QUATERNION_Y | AE_MOVIE_IMMUTABLE_QUATERNION_Z | AE_MOVIE_IMMUTABLE_QUATERNION_W,
	__AE_MOVIE_IMMUTABLE_END__
} aeMoviePropertyImmutableEnum;

typedef struct aeMovieLayerTransformation2DImuttable
{	
	float anchor_point_x;
	float anchor_point_y;
	float position_x;
	float position_y;
	float scale_x;
	float scale_y;
	float quaternion_z;
	float quaternion_w;	
}aeMovieLayerTransformation2DImuttable;

typedef struct aeMovieLayerTransformation3DImuttable
{	
	float anchor_point_x;
	float anchor_point_y;
	float anchor_point_z;
	float position_x;
	float position_y;
	float position_z;
	float scale_x;
	float scale_y;
	float scale_z;
	float quaternion_x;
	float quaternion_y;
	float quaternion_z;
	float quaternion_w;	
}aeMovieLayerTransformation3DImuttable;

typedef struct aeMovieLayerTransformation2DTimeline
{	
	const void * anchor_point_x;
	const void * anchor_point_y;
	const void * position_x;
	const void * position_y;
	const void * quaternion_z;
	const void * quaternion_w;
	const void * scale_x;
	const void * scale_y;	
}aeMovieLayerTransformation2DTimeline;

typedef struct aeMovieLayerTransformation3DTimeline
{	
	const void * anchor_point_x;
	const void * anchor_point_y;
	const void * anchor_point_z;
	const void * position_x;
	const void * position_y;
	const void * position_z;
	const void * quaternion_x;
	const void * quaternion_y;
	const void * quaternion_z;
	const void * quaternion_w;
	const void * scale_x;
	const void * scale_y;
	const void * scale_z;
}aeMovieLayerTransformation3DTimeline;

typedef struct aeMovieLayerTransformation
{
	uint32_t immutable_property_mask;

	float immutable_opacity;
	const void * timeline_opacity;
}aeMovieLayerTransformation;

typedef struct aeMovieLayerTransformation2D
{
	uint32_t immutable_property_mask;

	float immutable_opacity;
	const void * timeline_opacity;

	aeMovieLayerTransformation2DImuttable immutable;
	aeMovieLayerTransformation2DTimeline * timeline;
} aeMovieLayerTransformation2D;

typedef struct aeMovieLayerTransformation3D
{
	uint32_t immutable_property_mask;
	
	float immutable_opacity;
	const void * timeline_opacity;

	aeMovieLayerTransformation3DImuttable immutable;
	aeMovieLayerTransformation3DTimeline * timeline;
} aeMovieLayerTransformation3D;


ae_result_t ae_movie_load_layer_transformation( aeMovieStream * _stream, aeMovieLayerTransformation * _transformation, ae_bool_t _threeD );
void ae_movie_delete_layer_transformation( const aeMovieInstance * _instance, const aeMovieLayerTransformation * _transformation, ae_bool_t _threeD );
float ae_movie_make_layer_opacity( const aeMovieLayerTransformation * _transformation, ae_bool_t _threeD, uint32_t _index, ae_bool_t _interpolate, float _t );
void ae_movie_make_layer_transformation( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_bool_t _threeD, uint32_t _index, ae_bool_t _interpolate, float _t );

#	endif