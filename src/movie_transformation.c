#	include "movie_transformation.h"

#	include "movie_stream.h"
#	include "movie_math.h"

//////////////////////////////////////////////////////////////////////////
void * load_movie_layer_transformation_timeline( aeMovieStream * _stream, const char * _doc )
{
	(void)_doc;

	uint32_t zp_data_size;
	READ( _stream, zp_data_size );
	void * timeline = NEWV( _stream->instance, _doc, zp_data_size );
	READV( _stream, timeline, (size_t)zp_data_size );

	return timeline;
}
//////////////////////////////////////////////////////////////////////////
aeMovieResult load_movie_layer_transformation( aeMovieStream * _stream, aeMovieLayerTransformation * _transformation )
{
	uint32_t immutable_property_mask;
	READ( _stream, immutable_property_mask );
		
	_transformation->immutable_property_mask = immutable_property_mask;

	if( immutable_property_mask != __AE_MOVIE_IMMUTABLE_ALL__ )
	{
		aeMovieLayerTransformationTimeline * timeline = NEW( _stream->instance, aeMovieLayerTransformationTimeline );

		_transformation->timeline = timeline;
	}
	else
	{
		_transformation->timeline = AE_NULL;
	}

#	define AE_MOVIE_STREAM_PROPERTY(Mask, Name)\
	if( immutable_property_mask & Mask )\
	{\
		READ( _stream, _transformation->immuttable.Name );\
		if( _transformation->timeline != AE_NULL )\
		{\
			_transformation->timeline->Name = AE_NULL;\
		}\
	}\
	else\
	{\
		_transformation->immuttable.Name = 0.f;\
		_transformation->timeline->Name = load_movie_layer_transformation_timeline(_stream, #Name);\
	}

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_X, anchor_point_x );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Y, anchor_point_y );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Z, anchor_point_z );

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_X, position_x );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_Y, position_y );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_Z, position_z );

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_X, scale_x );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_Y, scale_y );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_Z, scale_z );

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_X, quaternion_x );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_Y, quaternion_y );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_Z, quaternion_z );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_W, quaternion_w );

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_OPACITY, opacity );

#	undef AE_MOVIE_STREAM_PROPERTY

	return AE_MOVIE_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void delete_movie_layer_transformation( const aeMovieInstance * _instance, const aeMovieLayerTransformation * _transformation )
{
	if( _transformation->timeline != AE_NULL )
	{
		aeMovieLayerTransformationTimeline * timeline = _transformation->timeline;

		DELETEN( _instance, timeline->anchor_point_x );
		DELETEN( _instance, timeline->anchor_point_y );
		DELETEN( _instance, timeline->anchor_point_z );
		DELETEN( _instance, timeline->position_x );
		DELETEN( _instance, timeline->position_y );
		DELETEN( _instance, timeline->position_z );
		DELETEN( _instance, timeline->quaternion_x );
		DELETEN( _instance, timeline->quaternion_y );
		DELETEN( _instance, timeline->quaternion_z );
		DELETEN( _instance, timeline->quaternion_w );
		DELETEN( _instance, timeline->scale_x );
		DELETEN( _instance, timeline->scale_y );
		DELETEN( _instance, timeline->scale_z );
		DELETEN( _instance, timeline->opacity );

		DELETE( _instance, _transformation->timeline );
	}
}
//////////////////////////////////////////////////////////////////////////
static float get_movie_layer_transformation_property( float _immutable, void * _property, uint32_t _index )
{
	if( _property == AE_NULL )
	{
		return _immutable;
	}

	uint32_t property_index = 0;

	uint32_t * property_uint32_t = (uint32_t *)_property;
	
	uint32_t zp_count = *(property_uint32_t++);

	for( uint32_t i = 0; i != zp_count; ++i )
	{
		uint32_t zp_block_type_count_data = *(property_uint32_t++);
		
		uint32_t zp_block_type = zp_block_type_count_data >> 24;
		uint32_t zp_block_count = zp_block_type_count_data & 0x00FFFFFF;

		if( property_index + zp_block_count > _index )
		{
			switch( zp_block_type )
			{
			case 0:
				{
					float block_value = *(float *)(property_uint32_t);

					return block_value;
				}break;
			case 1:
				{
					float block_begin = *(float *)(property_uint32_t++);
					float block_end = *(float *)(property_uint32_t);

					float block_add = (block_end - block_begin) / (float)(zp_block_count - 1);

					uint32_t block_index = _index - property_index;

					float block_value = block_begin + block_add * (float)block_index;

					return block_value;
				}break;
			case 3:
				{
					uint32_t block_index = _index - property_index;

					float block_value = ((float *)property_uint32_t)[block_index];

					return block_value;
				}break;
			}
		}
		else
		{
			switch( zp_block_type )
			{
			case 0:
				{
					property_uint32_t += 1;
				}break;
			case 1:
				{
					property_uint32_t += 2;
				}break;
			case 3:
				{
					property_uint32_t += zp_block_count;
				}break;
			}
		}

		property_index += zp_block_count;
	}

	return 0.f;
}
//////////////////////////////////////////////////////////////////////////
static float get_movie_layer_transformation_property_interpolate( float _immutable, void * _property, uint32_t _index, float _t )
{
	float data_0 = get_movie_layer_transformation_property( _immutable, _property, _index + 0 );
	float data_1 = get_movie_layer_transformation_property( _immutable, _property, _index + 1 );

	float data = linerp_f1( data_0, data_1, _t );

	return data;
}
//////////////////////////////////////////////////////////////////////////
float make_movie_layer_transformation( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, uint32_t _index, ae_bool_t _interpolate, float _t )
{
	float anchor_point[3];
	float position[3];
	float scale[3];
	float quaternion[4];
	float opacity;

	if( _interpolate == AE_TRUE )
	{
#	define AE_LINERP_PROPERTY( Name, OutName )\
	OutName = get_movie_layer_transformation_property_interpolate( _transformation->immuttable.Name, (_transformation->timeline == AE_NULL ? AE_NULL : _transformation->timeline->Name), _index, _t );

		AE_LINERP_PROPERTY( anchor_point_x, anchor_point[0] );
		AE_LINERP_PROPERTY( anchor_point_y, anchor_point[1] );
		AE_LINERP_PROPERTY( anchor_point_z, anchor_point[2] );

		AE_LINERP_PROPERTY( position_x, position[0] );
		AE_LINERP_PROPERTY( position_y, position[1] );
		AE_LINERP_PROPERTY( position_z, position[2] );

		AE_LINERP_PROPERTY( scale_x, scale[0] );
		AE_LINERP_PROPERTY( scale_y, scale[1] );
		AE_LINERP_PROPERTY( scale_z, scale[2] );

		AE_LINERP_PROPERTY( opacity, opacity );

#	undef AE_LINERP_PROPERTY

#	define AE_GET_PROPERTY_QUATERNION( Name, Index, OutName)\
		OutName = get_movie_layer_transformation_property( _transformation->immuttable.Name, (_transformation->timeline == AE_NULL ? AE_NULL : _transformation->timeline->Name), _index + Index );

		ae_quaternion_t q1;
		AE_GET_PROPERTY_QUATERNION( quaternion_x, 0, q1[0] );
		AE_GET_PROPERTY_QUATERNION( quaternion_y, 0, q1[1] );
		AE_GET_PROPERTY_QUATERNION( quaternion_z, 0, q1[2] );
		AE_GET_PROPERTY_QUATERNION( quaternion_w, 0, q1[3] );

		ae_quaternion_t q2;
		AE_GET_PROPERTY_QUATERNION( quaternion_x, 1, q2[0] );
		AE_GET_PROPERTY_QUATERNION( quaternion_y, 1, q2[1] );
		AE_GET_PROPERTY_QUATERNION( quaternion_z, 1, q2[2] );
		AE_GET_PROPERTY_QUATERNION( quaternion_w, 1, q2[3] );

		linerp_q( quaternion, q1, q2, _t );

#	undef AE_GET_PROPERTY_QUATERNION
	}
	else
	{
#	define AE_FIXED_PROPERTY( Name, OutName )\
	OutName = get_movie_layer_transformation_property( _transformation->immuttable.Name, (_transformation->timeline == AE_NULL ? AE_NULL : _transformation->timeline->Name), _index );

		AE_FIXED_PROPERTY( anchor_point_x, anchor_point[0] );
		AE_FIXED_PROPERTY( anchor_point_y, anchor_point[1] );
		AE_FIXED_PROPERTY( anchor_point_z, anchor_point[2] );

		AE_FIXED_PROPERTY( position_x, position[0] );
		AE_FIXED_PROPERTY( position_y, position[1] );
		AE_FIXED_PROPERTY( position_z, position[2] );

		AE_FIXED_PROPERTY( scale_x, scale[0] );
		AE_FIXED_PROPERTY( scale_y, scale[1] );
		AE_FIXED_PROPERTY( scale_z, scale[2] );

		AE_FIXED_PROPERTY( quaternion_x, quaternion[0] );
		AE_FIXED_PROPERTY( quaternion_y, quaternion[1] );
		AE_FIXED_PROPERTY( quaternion_z, quaternion[2] );
		AE_FIXED_PROPERTY( quaternion_w, quaternion[3] );

		AE_FIXED_PROPERTY( opacity, opacity );

#	undef AE_FIXED_PROPERTY
	}

	make_transformation_m4( _out, position, anchor_point, scale, quaternion );

	return opacity;
}