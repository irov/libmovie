#	include "movie/movie_node.h"

#	include "movie_memory.h"
#	include "movie_math.h"

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
		outName = linerp_f1( correct_rotate_from, correct_rotate_to, _t ); \
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
static void __setup_movie_node_relative( aeMovieNode * _nodes, uint32_t * _iterator, const aeMovieCompositionData * _compositionData, aeMovieNode * _parent )
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

		node->active = AE_FALSE;
		node->animate = AE_MOVIE_NODE_ANIMATE_STATIC;

		if( layer->parent_index == 0 )
		{
			node->relative = _parent;
		}

		switch( layer->type )
		{
		case AE_MOVIE_LAYER_TYPE_MOVIE:
			{
				__setup_movie_node_relative( _nodes, _iterator, layer->sub_composition, node );
			}break;
		case AE_MOVIE_LAYER_TYPE_SUB_MOVIE:
			{
				__setup_movie_node_relative( _nodes, _iterator, layer->sub_composition, node );
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
void __setup_movie_node_time( aeMovieNode * _nodes, uint32_t * _iterator, const aeMovieCompositionData * _compositionData, aeMovieNode * _parent, float _stretch )
{
	for( const aeMovieLayerData
		*it_layer = _compositionData->layers,
		*it_layer_end = _compositionData->layers + _compositionData->layer_count;
	it_layer != it_layer_end;
	++it_layer )
	{
		const aeMovieLayerData * layer = it_layer;

		aeMovieNode * node = _nodes + ((*_iterator)++);

		if( _parent == AE_NULL )
		{
			node->in_time = layer->in_time;
			node->out_time = layer->out_time;
			node->stretch = _stretch;
		}
		else
		{
			node->in_time = _parent->in_time + layer->in_time * _stretch;
			node->out_time = _parent->in_time + layer->out_time * _stretch;
			node->stretch = _stretch;
		}

		float to_stretch = _stretch * layer->stretch;

		switch( layer->type )
		{
		case AE_MOVIE_LAYER_TYPE_MOVIE:
			{
				__setup_movie_node_time( _nodes, _iterator, layer->sub_composition, node, to_stretch );
			}break;
		case AE_MOVIE_LAYER_TYPE_SUB_MOVIE:
			{
				__setup_movie_node_time( _nodes, _iterator, layer->sub_composition, node, to_stretch );
			}break;
		default:
			{
			}break;
		}
	}
}
//////////////////////////////////////////////////////////////////////////
aeMovieComposition * create_movie_composition( const aeMovieInstance * _instance, const aeMovieData * _data, const aeMovieCompositionData * _compositionData )
{
	aeMovieComposition * composition = NEW( _instance, aeMovieComposition );

	composition->movie_data = _data;
	composition->composition_data = _compositionData;

	composition->update_revision = 0;
	composition->timing = 0.f;
	composition->loop = AE_FALSE;

	uint32_t node_count = get_movie_composition_data_node_count( _data, _compositionData );
		
	composition->node_count = node_count;
	composition->nodes = NEWN( _instance, aeMovieNode, node_count );

	uint32_t node_relative_iterator = 0;

	__setup_movie_node_relative( composition->nodes, &node_relative_iterator, _compositionData, AE_NULL );

	uint32_t node_time_iterator = 0;

	__setup_movie_node_time( composition->nodes, &node_time_iterator, _compositionData, AE_NULL, 1.f );

	//for( aeMovieNode
	//	*it_node = composition->nodes,
	//	*it_node_end = composition->nodes + node_count;
	//it_node != it_node_end;
	//++it_node )
	//{
	//	aeMovieNode * node = it_node;

	//	float offset_time = __get_node_offset_time( node );

	//	node->in_time = offset_time + node->layer->in_time;
	//	node->out_time = offset_time + node->layer->out_time;
	//}

	return composition;
}
//////////////////////////////////////////////////////////////////////////
void destroy_movie_composition( const aeMovieInstance * _instance, const aeMovieComposition * _composition )
{
	DELETE( _instance, _composition->nodes );

	DELETE( _instance, _composition );
}
//////////////////////////////////////////////////////////////////////////
void set_movie_composition_loop( aeMovieComposition * _composition, ae_bool_t _loop )
{
	_composition->loop = _loop;
}
//////////////////////////////////////////////////////////////////////////
static void __update_node_matrix_fixed( aeMovieNode * _node, uint32_t _revision, uint32_t _frame )
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

				__update_node_matrix_fixed( node_relative, _revision, _frame );
			}

			ae_matrix4_t local_matrix;
			float local_opacity = __make_movie_layer_properties_fixed( local_matrix, _node->layer, _frame );

			mul_m4_m4( _node->matrix, node_relative->matrix, local_matrix );

			_node->opacity = node_relative->opacity * local_opacity;
		}
		else
		{
			_node->opacity = __make_movie_layer_properties_fixed( _node->matrix, _node->layer, _frame );
		}
	}
}
//////////////////////////////////////////////////////////////////////////
static void __update_node_matrix_interpolate( aeMovieNode * _node, uint32_t _revision, uint32_t _frame, float _t )
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

				__update_node_matrix_interpolate( node_relative, _revision, _frame, _t );
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
static void __update_node_matrix( aeMovieNode * _node, uint32_t _revision, uint32_t _frame, float _t, ae_bool_t _interpolate )
{
	if( _interpolate == AE_TRUE )
	{
		__update_node_matrix_interpolate( _node, _revision, _frame, _t );
	}
	else
	{
		__update_node_matrix_fixed( _node, _revision, _frame );
	}
}
//////////////////////////////////////////////////////////////////////////
void __update_movie_composition_node( aeMovieComposition * _composition, uint32_t _revision, uint32_t _beginFrame, uint32_t _endFrame )
{
	float frameDuration = _composition->composition_data->frameDuration;
	float frameDurationInv = 1.f / frameDuration;

	float end_timing = _composition->timing;

	for( aeMovieNode
		*it_node = _composition->nodes,
		*it_node_end = _composition->nodes + _composition->node_count;
	it_node != it_node_end;
	++it_node )
	{
		aeMovieNode * node = it_node;

		uint32_t indexIn = (uint32_t)(node->in_time * frameDurationInv);
		uint32_t indexOut = (uint32_t)(node->out_time * frameDurationInv);

		if( indexIn > _endFrame || indexOut < _beginFrame )
		{
			continue;
		}

		node->current_time = end_timing - node->in_time;

		uint32_t frameId = (uint32_t)((end_timing - node->in_time) / node->stretch * frameDurationInv);

		float t = end_timing / node->stretch * frameDurationInv - (float)frameId;

		if( _composition->loop == AE_TRUE && (node->layer->params & AE_MOVIE_LAYER_PARAM_LOOP) )
		{
			__update_node_matrix_interpolate( node, _revision, frameId, t );

			node->active = AE_TRUE;

			if( node->animate == AE_MOVIE_NODE_ANIMATE_STATIC )
			{
				node->animate = AE_MOVIE_NODE_ANIMATE_BEGIN;
			}
			else
			{
				node->animate = AE_MOVIE_NODE_ANIMATE_PROCESS;
			}
		}
		else
		{
			if( _beginFrame < indexIn && _endFrame >= indexIn && _endFrame < indexOut )
			{
				__update_node_matrix( node, _revision, frameId, t, (_endFrame + 1) < indexOut );

				node->active = AE_TRUE;
				node->animate = AE_MOVIE_NODE_ANIMATE_BEGIN;
			}
			else if( _endFrame >= indexOut && _beginFrame >= indexIn && _beginFrame < indexOut )
			{
				node->active = AE_FALSE;
				node->animate = AE_MOVIE_NODE_ANIMATE_END;
			}
			else if( _beginFrame >= indexIn && _endFrame >= indexIn && _endFrame < indexOut )
			{
				__update_node_matrix( node, _revision, frameId, t, (_endFrame + 1) < indexOut );

				if( node->active == AE_FALSE )
				{
					node->active = AE_TRUE;
					node->animate = AE_MOVIE_NODE_ANIMATE_BEGIN;
				}
				else
				{
					node->animate = AE_MOVIE_NODE_ANIMATE_PROCESS;
				}
			}
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

	float frameDuration = _composition->composition_data->frameDuration;

	uint32_t beginFrame = (uint32_t)(begin_timing / frameDuration);
	uint32_t endFrame = (uint32_t)(_composition->timing / frameDuration);

	float duration = _composition->composition_data->duration;

	while( _composition->timing > duration )
	{
		uint32_t lastFrame = (uint32_t)(duration / frameDuration);

		__update_movie_composition_node( _composition, update_revision, beginFrame, lastFrame );

		beginFrame = 0;

		_composition->timing -= duration;

		endFrame = (uint32_t)(_composition->timing / frameDuration);
	}
	
	__update_movie_composition_node( _composition, update_revision, beginFrame, endFrame );
}
//////////////////////////////////////////////////////////////////////////
static void __count_movie_redner_context( const aeMovieComposition * _composition, aeMovieRenderContext * _context )
{
	uint32_t mesh_count = 0;

	for( uint32_t iterator = 0; iterator != _composition->node_count; ++iterator )
	{
		const aeMovieNode * node = _composition->nodes + iterator;

		if( node->active == AE_FALSE )
		{
			continue;
		}

		const aeMovieLayerData * layer = node->layer;

		if( layer->renderable == AE_FALSE )
		{
			continue;
		}

		_context->render_node_indices[mesh_count++] = iterator;
	}

	_context->mesh_count = mesh_count;
}
//////////////////////////////////////////////////////////////////////////
void begin_movie_render_context( const aeMovieComposition * _composition, aeMovieRenderContext * _context )
{
	_context->composition = _composition;
	
	float * sprite_uv = _context->sprite_uv;

	*sprite_uv++ = 0.f;
	*sprite_uv++ = 0.f;
	*sprite_uv++ = 1.f;
	*sprite_uv++ = 0.f;
	*sprite_uv++ = 1.f;
	*sprite_uv++ = 1.f;
	*sprite_uv++ = 0.f;
	*sprite_uv++ = 1.f;

	uint16_t * sprite_indices = _context->sprite_indices;
	
	*sprite_indices++ = 0;
	*sprite_indices++ = 3;
	*sprite_indices++ = 1;
	*sprite_indices++ = 1;
	*sprite_indices++ = 3;
	*sprite_indices++ = 2;

	__count_movie_redner_context( _composition, _context );
}
//////////////////////////////////////////////////////////////////////////
static void __make_sprite_vertices( const aeMovieRenderContext * _context, float _offset_x, float _offset_y, float _width, float _height, const ae_matrix4_t _matrix, aeMovieRenderMesh * _mesh )
{
	float v_position[8];

	float * v = v_position;

	*v++ = _offset_x + _width * 0.f;
	*v++ = _offset_y + _height * 0.f;
	*v++ = _offset_x + _width * 1.f;
	*v++ = _offset_y + _height * 0.f;
	*v++ = _offset_x + _width * 1.f;
	*v++ = _offset_y + _height * 1.f;
	*v++ = _offset_x + _width * 0.f;
	*v++ = _offset_y + _height * 1.f;

	_mesh->vertexCount = 4;
	_mesh->indexCount = 6;

	mul_v3_v2_m4( _mesh->position + 0, v_position + 0, _matrix );
	mul_v3_v2_m4( _mesh->position + 3, v_position + 2, _matrix );
	mul_v3_v2_m4( _mesh->position + 6, v_position + 4, _matrix );
	mul_v3_v2_m4( _mesh->position + 9, v_position + 6, _matrix );

	_mesh->uv = _context->sprite_uv;

	_mesh->indices = _context->sprite_indices;
}
//////////////////////////////////////////////////////////////////////////
void compute_movie_mesh( const aeMovieRenderContext * _context, uint32_t _index, aeMovieRenderMesh * _vertices )
{
	const aeMovieComposition * composition = _context->composition;
	uint32_t node_index = _context->render_node_indices[_index];

	const aeMovieNode * node = composition->nodes + node_index;

	const aeMovieLayerData * layer = node->layer;

	uint8_t layer_type = layer->type;

	_vertices->layer_type = layer_type;

	_vertices->animate = node->animate;
	
	switch( layer_type )
	{
	case AE_MOVIE_LAYER_TYPE_SLOT:
		{
			
		}break;
	case AE_MOVIE_LAYER_TYPE_SOLID:
		{
			aeMovieResourceSolid * resource_solid = (aeMovieResourceSolid *)layer->resource;

			_vertices->resource_type = resource_solid->base.type;
			_vertices->resource_data = resource_solid->base.data;

			float width = resource_solid->width;
			float height = resource_solid->height;

			__make_sprite_vertices( _context, 0.f, 0.f, width, height, node->matrix, _vertices );

			_vertices->r = resource_solid->r;
			_vertices->g = resource_solid->g;
			_vertices->b = resource_solid->b;
			_vertices->a = node->opacity;
		}break;
	case AE_MOVIE_LAYER_TYPE_SEQUENCE:
		{
			aeMovieResourceSequence * resource_sequence = (aeMovieResourceSequence *)layer->resource;

			float frameDuration = resource_sequence->frameDuration;

			uint32_t frame_sequence = (uint32_t)(node->current_time / frameDuration);

			aeMovieResourceImage * resource_image = resource_sequence->images[frame_sequence];

			_vertices->resource_type = resource_image->base.type;
			_vertices->resource_data = resource_image->base.data;

			float offset_x = resource_image->offset_x;
			float offset_y = resource_image->offset_y;

			float width = resource_image->width;
			float height = resource_image->height;

			__make_sprite_vertices( _context, offset_x, offset_y, width, height, node->matrix, _vertices );

			_vertices->r = 1.f;
			_vertices->g = 1.f;
			_vertices->b = 1.f;
			_vertices->a = node->opacity;
		}break;
	case AE_MOVIE_LAYER_TYPE_VIDEO:
		{
			
		}break;
	case AE_MOVIE_LAYER_TYPE_PARTICLE:
		{
			
		}break;
	case AE_MOVIE_LAYER_TYPE_IMAGE:
		{
			aeMovieResourceImage * resource_image = (aeMovieResourceImage *)layer->resource;

			_vertices->resource_type = resource_image->base.type;
			_vertices->resource_data = resource_image->base.data;

			float offset_x = resource_image->offset_x;
			float offset_y = resource_image->offset_y;

			float width = resource_image->width;
			float height = resource_image->height;

			__make_sprite_vertices( _context, offset_x, offset_y, width, height, node->matrix, _vertices );
						
			_vertices->r = 1.f;
			_vertices->g = 1.f;
			_vertices->b = 1.f;
			_vertices->a = node->opacity;
		}break;
	}
}
//////////////////////////////////////////////////////////////////////////