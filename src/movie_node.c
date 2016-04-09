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
static aeMovieNode * __find_node_by_layer( aeMovieNode * _nodes, uint32_t _begin, uint32_t _end, const aeMovieLayerData * _layer )
{
	for( aeMovieNode
		*it_node = _nodes + _begin,
		*it_node_end = _nodes + _end;
	it_node != it_node_end;
	++it_node )
	{
		aeMovieNode * node = it_node;

		const aeMovieLayerData * node_layer = node->layer;

		if( node_layer == _layer )
		{
			return node;
		}
	}

	return AE_NULL;
}
//////////////////////////////////////////////////////////////////////////
static const aeMovieLayerData * __find_layer_by_index( const aeMovieCompositionData * _compositionData, uint32_t _index )
{
	for( const aeMovieLayerData
		*it_layer = _compositionData->layers,
		*it_layer_end = _compositionData->layers + _compositionData->layer_count;
	it_layer != it_layer_end;
	++it_layer )
	{
		const aeMovieLayerData * layer = it_layer;

		uint32_t layer_index = layer->index;

		if( layer_index == _index )
		{
			return layer;
		}
	}

	return AE_NULL;
}
//////////////////////////////////////////////////////////////////////////
static void __setup_movie_node( aeMovieNode * _nodes, uint32_t * _iterator, const aeMovieCompositionData * _compositionData, aeMovieNode * _parent )
{
	uint32_t begin_index = *_iterator;

	for( const aeMovieLayerData
		*it_layer = _compositionData->layers,
		*it_layer_end = _compositionData->layers + _compositionData->layer_count;
	it_layer != it_layer_end;
	++it_layer )
	{
		const aeMovieLayerData * layer = it_layer;

		aeMovieNode * node = _nodes + ((*_iterator)++);

		node->layer = layer;
		node->matrix_revision = 0;

		if( layer->parent_index == 0 )
		{
			node->relative = _parent;
		}

		switch( layer->type )
		{
		case AE_MOVIE_LAYER_TYPE_MOVIE:
			{
				__setup_movie_node( _nodes, _iterator, layer->sub_composition, node );
			}break;
		case AE_MOVIE_LAYER_TYPE_SUB_MOVIE:
			{
				__setup_movie_node( _nodes, _iterator, layer->sub_composition, node );
			}break;
		default:
			{
			}break;
		}
	}

	uint32_t end_index = *_iterator;
	
	for( const aeMovieLayerData
		*it_layer = _compositionData->layers,
		*it_layer_end = _compositionData->layers + _compositionData->layer_count;
	it_layer != it_layer_end;
	++it_layer )
	{
		const aeMovieLayerData * layer = it_layer;

		uint32_t parent_index = layer->parent_index;

		if( parent_index == 0 )
		{
			continue;
		}

		aeMovieNode * node = __find_node_by_layer( _nodes, begin_index, end_index, layer );

		const aeMovieLayerData * parent_layer = __find_layer_by_index( _compositionData, parent_index );

		aeMovieNode * parent_node = __find_node_by_layer( _nodes, begin_index, end_index, parent_layer );

		node->relative = parent_node;
	}
}
//////////////////////////////////////////////////////////////////////////
static const aeMovieNode * __get_parent_node_composition( const aeMovieNode * _node )
{
	if( _node->relative == AE_NULL )
	{
		return AE_NULL;
	}

	if( _node->relative->layer->sub_composition != AE_NULL )
	{
		return _node->relative;
	}

	return __get_parent_node_composition( _node->relative );
}
//////////////////////////////////////////////////////////////////////////
static float __get_node_offset_time( const aeMovieNode * _node )
{
	const aeMovieNode * node_composition = __get_parent_node_composition( _node );

	if( node_composition == AE_NULL )
	{
		return 0.f;
	}

	float composition_time = __get_node_offset_time( node_composition );

	return composition_time;
}
//////////////////////////////////////////////////////////////////////////
aeMovieComposition * create_movie_composition( const aeMovieInstance * _instance, const aeMovieData * _data, const aeMovieCompositionData * _compositionData )
{
	aeMovieComposition * composition = NEW( _instance, aeMovieComposition );

	composition->data = _data;
	composition->composition_data = _compositionData;

	composition->update_revision = 0;
	composition->timing = 0.f;

	uint32_t node_count = get_movie_composition_data_node_count( _data, _compositionData );
		
	composition->node_count = node_count;
	composition->nodes = NEWN( _instance, aeMovieNode, node_count );

	uint32_t node_iterator = 0;

	__setup_movie_node( composition->nodes, &node_iterator, _compositionData, AE_NULL );

	for( aeMovieNode
		*it_node = composition->nodes,
		*it_node_end = composition->nodes + node_count;
	it_node != it_node_end;
	++it_node )
	{
		aeMovieNode * node = it_node;

		float offset_time = __get_node_offset_time( node );

		node->in_time = offset_time + node->layer->in_time;
		node->out_time = offset_time + node->layer->out_time;
	}

	return composition;
}
//////////////////////////////////////////////////////////////////////////
void delete_movie_node( const aeMovieInstance * _instance, const aeMovieNode * _node )
{
	DELETE( _instance, _node );
}
//////////////////////////////////////////////////////////////////////////
static void __update_node_matrix( aeMovieNode * _node, uint32_t _revision, uint32_t _frame, float _t )
{
	if( _node->matrix_revision != _revision )
	{
		_node->matrix_revision = _revision;

		if( _node->relative != AE_NULL )
		{
			aeMovieNode * node_relative = _node->relative;

			if( node_relative->matrix_revision != _revision )
			{
				node_relative->matrix_revision = _revision;

				__update_node_matrix( node_relative, _revision, _frame, _t );
			}

			ae_matrix4_t local_matrix;
			float local_opacity = __make_movie_layer_properties_interpolate( local_matrix, _node->layer, _frame, _t );

			mul_m4_m4( _node->matrix, node_relative->matrix, local_matrix );

			_node->opacity = node_relative->opacity * local_opacity;
		}
		else
		{
			_node->opacity = __make_movie_layer_properties_interpolate( _node->matrix, _node->layer, _frame, _t );
		}
	}
}
//////////////////////////////////////////////////////////////////////////
void update_movie_composition( aeMovieComposition * _composition, float _timing )
{
	_composition->update_revision++;
	uint32_t update_revision = _composition->update_revision;

	float begin_timing = _composition->timing;

	_composition->timing += _timing;

	float end_timing = _composition->timing;

	float frameDuration = _composition->composition_data->frameDuration;

	uint32_t begin_frame = (uint32_t)(begin_timing / _composition->composition_data->frameDuration);
	uint32_t end_frame = (uint32_t)(end_timing / _composition->composition_data->frameDuration);

	float delta_timing = end_timing - (float)(end_frame * frameDuration);
	
	float t = delta_timing / frameDuration;

	for( aeMovieNode
		*it_node = _composition->nodes,
		*it_node_end = _composition->nodes + _composition->node_count;
	it_node != it_node_end;
	++it_node )
	{
		aeMovieNode * node = it_node;

		__update_node_matrix( node, update_revision, end_frame, t );
	}
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