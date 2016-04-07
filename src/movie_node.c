#	include "movie/movie_node.h"

#	include "memory.h"
#	include "math.h"

//////////////////////////////////////////////////////////////////////////
static float __linerp_f1( float _in1, float _in2, float _scale )
{
	return _in1 + (_in2 - _in1) * _scale;
}
//////////////////////////////////////////////////////////////////////////
static float __make_movie_layer_properties_fixed( ae_matrix4_t _out, const aeMovieLayerData * _layer, uint32_t _index )
{
	float anchor_point[3];
	float position[3];
	float scale[3];
	float rotation[3];
	float opacity;

#	define AE_LINERP_PROPERTY( Mask, immutableName, propertyName, outName )\
	if( _layer->immutable_property_mask & Mask )\
	{\
		outName = _layer->immutableName;\
	}\
	else\
	{\
		outName = _layer->propertyName[_index];\
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

	AE_LINERP_PROPERTY( AE_MOVIE_IMMUTABLE_ROTATION_X, immuttable_rotation_x, property_rotation_x, rotation[0] );
	AE_LINERP_PROPERTY( AE_MOVIE_IMMUTABLE_ROTATION_Y, immuttable_rotation_y, property_rotation_y, rotation[1] );
	AE_LINERP_PROPERTY( AE_MOVIE_IMMUTABLE_ROTATION_Z, immuttable_rotation_z, property_rotation_z, rotation[2] );

	AE_LINERP_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_Z, immuttable_scale_z, property_scale_z, opacity );

#	undef AE_LINERP_PROPERTY

	make_transformation_m4( _out, position, anchor_point, scale, rotation );

	return opacity;
}
//////////////////////////////////////////////////////////////////////////
static float __make_movie_layer_properties_interpolate( ae_matrix4_t _out, const aeMovieLayerData * _layer, uint32_t _index, float _t )
{
	float anchor_point[3];
	float position[3];
	float scale[3];
	float rotation[3];
	float opacity;

#	define AE_LINERP_PROPERTY( Mask, immutableName, propertyName, outName )\
	if( _layer->immutable_property_mask & Mask )\
			{\
		outName = _layer->immutableName;\
			}\
				else\
			{\
		float value0 = _layer->propertyName[_index + 0];\
		float value1 = _layer->propertyName[_index + 1];\
		outName = __linerp_f1( value0, value1, _t );\
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

	AE_LINERP_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_Z, immuttable_scale_z, property_scale_z, opacity );

#	undef AE_LINERP_PROPERTY

#	define AE_LINERP_PROPERTY2( Mask, immutableName, propertyName, outName )\
	if( _layer->immutable_property_mask & Mask )\
			{\
		outName = _layer->immutableName; \
			}\
				else\
			{\
		float value0 = _layer->propertyName[_index + 0];\
		float value1 = _layer->propertyName[_index + 1];\
		float correct_rotate_from = angle_norm( value0 );\
		float correct_rotate_to = angle_correct_interpolate_from_to( correct_rotate_from, value1 );\
		outName = __linerp_f1( correct_rotate_from, correct_rotate_to, _t ); \
			}

	AE_LINERP_PROPERTY2( AE_MOVIE_IMMUTABLE_ROTATION_X, immuttable_rotation_x, property_rotation_x, rotation[0] );
	AE_LINERP_PROPERTY2( AE_MOVIE_IMMUTABLE_ROTATION_Y, immuttable_rotation_y, property_rotation_y, rotation[1] );
	AE_LINERP_PROPERTY2( AE_MOVIE_IMMUTABLE_ROTATION_Z, immuttable_rotation_z, property_rotation_z, rotation[2] );

#	undef AE_LINERP_PROPERTY2

	make_transformation_m4( _out, position, anchor_point, scale, rotation );

	return opacity;
}
//////////////////////////////////////////////////////////////////////////
aeMovieNode * create_movie_node( const aeMovieInstance * _instance, const aeMovieData * _data, const aeMovieCompositionData * _composition )
{
	aeMovieNode * node = NEW( _instance, aeMovieNode );

	//node->data = _data;
	//node->composition = _composition;

	//node->timing = 0.f;

	return node;
}
//////////////////////////////////////////////////////////////////////////
void delete_movie_node( const aeMovieInstance * _instance, const aeMovieNode * _node )
{
	DELETE( _instance, _node );
}
//////////////////////////////////////////////////////////////////////////
void update_movie_node( const aeMovieInstance * _instance, const aeMovieNode * _node )
{

}
//////////////////////////////////////////////////////////////////////////
void compute_movie_vertices( const aeMovieInstance * _instance, const aeMovieNode * _node, const aeMovieLayerData * _layer, aeMovieRender * _render )
{
	//for( const aeMovieCompositionData
	//	*it_composition = _movie->compositions,
	//	*it_composition_end = _movie->compositions + _movie->composition_count;
	//it_composition != it_composition_end;
	//++it_composition )
	//{
	//	const aeMovieCompositionData * composition = it_composition;

	//	if( ae_strcmp( composition->name, _name ) != 0 )
	//	{
	//		continue;
	//	}

	//	return composition;
	//}
}
//////////////////////////////////////////////////////////////////////////