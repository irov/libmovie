#	include "movie_transformation.h"

#	include "movie_stream.h"
#	include "movie_math.h"

//////////////////////////////////////////////////////////////////////////
static aeMovieResult __load_movie_data_layer_property_zp( const aeMovieStream * _stream, float * _values )
{
	uint32_t count = READZ( _stream );

	float * stream_values = _values;

	for( uint32_t i = 0; i != count; ++i )
	{
		uint8_t block_type;
		READ( _stream, block_type );

		uint32_t block_count = READZ( _stream );

		switch( block_type )
		{
		case 0:
			{
				float block_value;
				READ( _stream, block_value );

				for( uint32_t block_index = 0; block_index != block_count; ++block_index )
				{
					*stream_values++ = block_value;
				}
			}break;
		case 1:
			{
				float block_begin;
				READ( _stream, block_begin );

				float block_end;
				READ( _stream, block_end );

				float block_add = (block_end - block_begin) / (float)(block_count - 1);

				for( uint32_t block_index = 0; block_index != block_count; ++block_index )
				{
					float block_value = block_begin + block_add * block_index;

					*stream_values++ = block_value;
				}
			}break;
		case 3:
			{
				for( uint32_t block_index = 0; block_index != block_count; ++block_index )
				{
					float block_value;
					READ( _stream, block_value );

					*stream_values++ = block_value;
				}
			}break;
		default:
			{
				return AE_MOVIE_FAILED;
			}break;
		}
	}

	return AE_MOVIE_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
aeMovieResult load_movie_layer_transformation( const aeMovieInstance * _instance, const aeMovieStream * _stream, aeMovieLayerTransformation * _transformation, uint32_t _count )
{
	uint32_t immutable_property_mask;
	READ( _stream, immutable_property_mask );

	_transformation->immutable_property_mask = immutable_property_mask;

#	define AE_MOVIE_STREAM_PROPERTY(Mask, ImmutableName, Name)\
	if( immutable_property_mask & Mask )\
					{\
		READ( _stream, _transformation->ImmutableName );\
		_transformation->Name = AE_NULL;\
					}\
					else\
					{\
		_transformation->ImmutableName = 0.f;\
		_transformation->Name = NEWN( _instance, float, _count );\
		if( __load_movie_data_layer_property_zp( _stream, _transformation->Name ) == AE_MOVIE_FAILED ) return AE_MOVIE_FAILED;\
					}

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_X, immuttable_anchor_point_x, property_anchor_point_x );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Y, immuttable_anchor_point_y, property_anchor_point_y );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Z, immuttable_anchor_point_z, property_anchor_point_z );

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_X, immuttable_position_x, property_position_x );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_Y, immuttable_position_y, property_position_y );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_Z, immuttable_position_z, property_position_z );

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_X, immuttable_scale_x, property_scale_x );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_Y, immuttable_scale_y, property_scale_y );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_Z, immuttable_scale_z, property_scale_z );

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_X, immuttable_quaternion_x, property_quaternion_x );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_Y, immuttable_quaternion_y, property_quaternion_y );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_Z, immuttable_quaternion_z, property_quaternion_z );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_W, immuttable_quaternion_w, property_quaternion_w );

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_OPACITY, immuttable_opacity, property_opacity );

#	undef AE_MOVIE_STREAM_PROPERTY

	return AE_MOVIE_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
void delete_movie_layer_transformation( const aeMovieInstance * _instance, const aeMovieLayerTransformation * _transformation )
{
	DELETEN( _instance, _transformation->property_anchor_point_x );
	DELETEN( _instance, _transformation->property_anchor_point_y );
	DELETEN( _instance, _transformation->property_anchor_point_z );
	DELETEN( _instance, _transformation->property_position_x );
	DELETEN( _instance, _transformation->property_position_y );
	DELETEN( _instance, _transformation->property_position_z );
	DELETEN( _instance, _transformation->property_scale_x );
	DELETEN( _instance, _transformation->property_scale_y );
	DELETEN( _instance, _transformation->property_scale_z );
	DELETEN( _instance, _transformation->property_quaternion_x );
	DELETEN( _instance, _transformation->property_quaternion_y );
	DELETEN( _instance, _transformation->property_quaternion_z );
	DELETEN( _instance, _transformation->property_quaternion_w );
	DELETEN( _instance, _transformation->property_opacity );
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
#	define AE_LINERP_PROPERTY( Mask, immutableName, propertyName, outName )\
	if( _transformation->immutable_property_mask & Mask ){\
		outName = _transformation->immutableName;\
	}else{\
		float value0 = _transformation->propertyName[_index + 0];\
		float value1 = _transformation->propertyName[_index + 1];\
		outName = linerp_f1( value0, value1, _t );\
		}

		AE_LINERP_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_X, immuttable_anchor_point_x, property_anchor_point_x, anchor_point[0] );
		AE_LINERP_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Y, immuttable_anchor_point_y, property_anchor_point_y, anchor_point[1] );
		AE_LINERP_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Z, immuttable_anchor_point_z, property_anchor_point_z, anchor_point[2] );

		AE_LINERP_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_X, immuttable_position_x, property_position_x, position[0] );
		AE_LINERP_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_Y, immuttable_position_y, property_position_y, position[1] );
		AE_LINERP_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_Z, immuttable_position_z, property_position_z, position[2] );

		AE_LINERP_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_X, immuttable_scale_x, property_scale_x, scale[0] );
		AE_LINERP_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_Y, immuttable_scale_y, property_scale_y, scale[1] );
		AE_LINERP_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_Z, immuttable_scale_z, property_scale_z, scale[2] );

		AE_LINERP_PROPERTY( AE_MOVIE_IMMUTABLE_OPACITY, immuttable_opacity, property_opacity, opacity );

#	undef AE_LINERP_PROPERTY

#	define AE_GET_PROPERTY_QUATERNION( Mask, immutableName, propertyName, Index )\
		((_transformation->immutable_property_mask & Mask) ? _transformation->immutableName : _transformation->propertyName[_index + Index])

		ae_quaternion_t q1;
		q1[0] = AE_GET_PROPERTY_QUATERNION( AE_MOVIE_IMMUTABLE_QUATERNION_X, immuttable_quaternion_x, property_quaternion_x, 0 );
		q1[1] = AE_GET_PROPERTY_QUATERNION( AE_MOVIE_IMMUTABLE_QUATERNION_Y, immuttable_quaternion_y, property_quaternion_y, 0 );
		q1[2] = AE_GET_PROPERTY_QUATERNION( AE_MOVIE_IMMUTABLE_QUATERNION_Z, immuttable_quaternion_z, property_quaternion_z, 0 );
		q1[3] = AE_GET_PROPERTY_QUATERNION( AE_MOVIE_IMMUTABLE_QUATERNION_W, immuttable_quaternion_w, property_quaternion_w, 0 );

		ae_quaternion_t q2;
		q2[0] = AE_GET_PROPERTY_QUATERNION( AE_MOVIE_IMMUTABLE_QUATERNION_X, immuttable_quaternion_x, property_quaternion_x, 1 );
		q2[1] = AE_GET_PROPERTY_QUATERNION( AE_MOVIE_IMMUTABLE_QUATERNION_Y, immuttable_quaternion_y, property_quaternion_y, 1 );
		q2[2] = AE_GET_PROPERTY_QUATERNION( AE_MOVIE_IMMUTABLE_QUATERNION_Z, immuttable_quaternion_z, property_quaternion_z, 1 );
		q2[3] = AE_GET_PROPERTY_QUATERNION( AE_MOVIE_IMMUTABLE_QUATERNION_W, immuttable_quaternion_w, property_quaternion_w, 1 );

		linerp_q( quaternion, q1, q2, _t );

#	undef AE_GET_PROPERTY_QUATERNION
	}
	else
	{
#	define AE_FIXED_PROPERTY( Mask, immutableName, propertyName, outName )\
	if( _transformation->immutable_property_mask & Mask )\
		{\
			outName = _transformation->immutableName;\
		}\
		else\
		{\
			outName = _transformation->propertyName[_index];\
		}

		AE_FIXED_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_X, immuttable_anchor_point_x, property_anchor_point_x, anchor_point[0] );
		AE_FIXED_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Y, immuttable_anchor_point_y, property_anchor_point_y, anchor_point[1] );
		AE_FIXED_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Z, immuttable_anchor_point_z, property_anchor_point_z, anchor_point[2] );

		AE_FIXED_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_X, immuttable_position_x, property_position_x, position[0] );
		AE_FIXED_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_Y, immuttable_position_y, property_position_y, position[1] );
		AE_FIXED_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_Z, immuttable_position_z, property_position_z, position[2] );

		AE_FIXED_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_X, immuttable_scale_x, property_scale_x, scale[0] );
		AE_FIXED_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_Y, immuttable_scale_y, property_scale_y, scale[1] );
		AE_FIXED_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_Z, immuttable_scale_z, property_scale_z, scale[2] );

		AE_FIXED_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_X, immuttable_quaternion_x, property_quaternion_x, quaternion[0] );
		AE_FIXED_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_Y, immuttable_quaternion_y, property_quaternion_y, quaternion[1] );
		AE_FIXED_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_Z, immuttable_quaternion_z, property_quaternion_z, quaternion[2] );
		AE_FIXED_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_W, immuttable_quaternion_w, property_quaternion_w, quaternion[3] );

		AE_FIXED_PROPERTY( AE_MOVIE_IMMUTABLE_OPACITY, immuttable_opacity, property_opacity, opacity );

#	undef AE_FIXED_PROPERTY
	}

	make_transformation_m4( _out, position, anchor_point, scale, quaternion );

	return opacity;
}