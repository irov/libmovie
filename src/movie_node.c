#	include "movie/movie_node.h"

#	include "movie_transformation.h"
#	include "movie_memory.h"
#	include "movie_math.h"

#	include "movie_struct.h"

#	ifdef _DEBUG
#	ifndef AE_MOVIE_NO_DEBUG
#	ifndef AE_MOVIE_DEBUG
#	define AE_MOVIE_DEBUG
#	endif
#	endif
#	endif

//////////////////////////////////////////////////////////////////////////
typedef enum
{
	AE_MOVIE_NODE_ANIMATE_STATIC,
	AE_MOVIE_NODE_ANIMATE_BEGIN,
	AE_MOVIE_NODE_ANIMATE_PROCESS,
	AE_MOVIE_NODE_ANIMATE_END,
	__AE_MOVIE_NODE_ANIMATE_STATES__
} aeMovieNodeAnimationStateEnum;
//////////////////////////////////////////////////////////////////////////
static aeMovieNode * __find_node_by_layer( aeMovieNode * _nodes, uint32_t _begin, uint32_t _end, const aeMovieLayerData * _layer )
{
	aeMovieNode * it_node = _nodes + _begin;
	aeMovieNode * it_node_end = _nodes + _end;
	for( ; it_node != it_node_end; ++it_node )
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
	const aeMovieLayerData * it_layer = _compositionData->layers;
	const aeMovieLayerData * it_layer_end = _compositionData->layers + _compositionData->layer_count;
	for( ; it_layer != it_layer_end; ++it_layer )
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
static float __compute_movie_color_r(const aeMovieLayerColorVertex * _colorVertex, uint32_t _frame, ae_bool_t _interpolate, float t)
{
    if( _colorVertex->immutable_r == AE_TRUE )
    {
        return _colorVertex->immutable_color_vertex_r;
    }

    if( _interpolate == AE_FALSE )
    {
        ae_color_t c = _colorVertex->color_vertites_r[_frame];

        float cf = tof_c(c);

        return cf;
    }

    ae_color_t c0 = _colorVertex->color_vertites_r[_frame + 0];
    ae_color_t c1 = _colorVertex->color_vertites_r[_frame + 1];

    float cf = linerp_c(c0, c1, t);

    return cf;
}
//////////////////////////////////////////////////////////////////////////
static float __compute_movie_color_g(const aeMovieLayerColorVertex * _colorVertex, uint32_t _frame, ae_bool_t _interpolate, float t)
{
    if( _colorVertex->immutable_g == AE_TRUE )
    {
        return _colorVertex->immutable_color_vertex_g;
    }

    if( _interpolate == AE_FALSE )
    {
        ae_color_t c = _colorVertex->color_vertites_g[_frame];

        float cf = tof_c(c);

        return cf;
    }

    ae_color_t c0 = _colorVertex->color_vertites_g[_frame + 0];
    ae_color_t c1 = _colorVertex->color_vertites_g[_frame + 1];

    float cf = linerp_c(c0, c1, t);

    return cf;
}
//////////////////////////////////////////////////////////////////////////
static float __compute_movie_color_b(const aeMovieLayerColorVertex * _colorVertex, uint32_t _frame, ae_bool_t _interpolate, float t)
{
    if( _colorVertex->immutable_b == AE_TRUE )
    {
        return _colorVertex->immutable_color_vertex_b;
    }

    if( _interpolate == AE_FALSE )
    {
        ae_color_t c = _colorVertex->color_vertites_b[_frame];

        float cf = tof_c(c);

        return cf;
    }

    ae_color_t c0 = _colorVertex->color_vertites_b[_frame + 0];
    ae_color_t c1 = _colorVertex->color_vertites_b[_frame + 1];

    float cf = linerp_c(c0, c1, t);

    return cf;
}
//////////////////////////////////////////////////////////////////////////
#	ifdef AE_MOVIE_DEBUG
static ae_bool_t __test_error_composition_layer_frame( const aeMovieInstance * _instance, const aeMovieCompositionData * _compositionData, const aeMovieLayerData * _layerData, uint32_t _frameId, const char * _msg )
{
	if( _frameId >= _layerData->frame_count )
	{
		_instance->logerror( _instance->instance_data
			, AE_ERROR_INTERNAL
			, _compositionData->name
			, _layerData->name
			, _msg
			);

		return AE_FALSE;
	}

	return AE_TRUE;
}
#	endif
//////////////////////////////////////////////////////////////////////////
static void __update_movie_composition_node_matrix( aeMovieComposition * _composition, aeMovieNode * _node, uint32_t _revision, uint32_t _frameId, ae_bool_t _interpolate, float _t )
{
	if( _node->matrix_revision == _revision )
	{
		return;
	}

#	ifdef AE_MOVIE_DEBUG
	if( __test_error_composition_layer_frame( _composition->movie_data->instance
		, _composition->composition_data
		, _node->layer
		, _frameId
		, "__update_movie_composition_node_matrix frame id out count"
		) == AE_FALSE )
	{
		return;
	}
#	endif

	_node->matrix_revision = _revision;

    float local_r = 1.f;
    float local_g = 1.f;
    float local_b = 1.f;

    if( _node->layer->color_vertex != AE_NULL )
    {
        local_r = __compute_movie_color_r(_node->layer->color_vertex, _frameId, _interpolate, _t);
        local_g = __compute_movie_color_g(_node->layer->color_vertex, _frameId, _interpolate, _t);
        local_b = __compute_movie_color_b(_node->layer->color_vertex, _frameId, _interpolate, _t);
    }

	if( _node->relative == AE_NULL )
	{
		float local_opacity = make_movie_layer_transformation( _node->matrix, _node->layer->transformation, _frameId, _interpolate, _t );

		if( _node->layer->sub_composition != AE_NULL )
		{
			_node->composition_opactity = local_opacity;
            
            _node->composition_r = local_r;
            _node->composition_g = local_g;
            _node->composition_b = local_b;
        }
		else
		{
			_node->composition_opactity = 1.f;

            _node->composition_r = 1.f;
            _node->composition_g = 1.f;
            _node->composition_b = 1.f;
		}

		_node->opacity = local_opacity;

        _node->r = local_r;
        _node->g = local_g;
        _node->b = local_b;

		return;
	}

	aeMovieNode * node_relative = _node->relative;

	if( node_relative->matrix_revision != _revision )
	{
		float composition_time = _composition->time;

		float frameDurationInv = node_relative->layer->composition->frameDurationInv;

		float current_time = composition_time - node_relative->in_time + node_relative->start_time;

		float frame_time = current_time / node_relative->stretch * frameDurationInv;

		if( frame_time < 0.f )
		{
			frame_time = 0.f;
		}

		uint32_t frame_relative = (uint32_t)frame_time;

		float t_relative = frame_time - (float)frame_relative;

		__update_movie_composition_node_matrix( _composition, node_relative, _revision, frame_relative, _interpolate, t_relative );
	}

	ae_matrix4_t local_matrix;
	float local_opacity = make_movie_layer_transformation( local_matrix, _node->layer->transformation, _frameId, _interpolate, _t );

	mul_m4_m4( _node->matrix, local_matrix, node_relative->matrix );

	if( _node->layer->sub_composition != AE_NULL )
	{
		_node->composition_opactity = node_relative->composition_opactity * local_opacity;
        
        _node->composition_r = node_relative->composition_r * local_r;
        _node->composition_g = node_relative->composition_g * local_g;
        _node->composition_b = node_relative->composition_b * local_b;
	}
	else
	{
		_node->composition_opactity = node_relative->composition_opactity;
        
        _node->composition_r = node_relative->composition_r;
        _node->composition_g = node_relative->composition_g;
        _node->composition_b = node_relative->composition_b;
	}

	_node->opacity = node_relative->composition_opactity * local_opacity;
    
    _node->r = node_relative->composition_r * local_r;
    _node->g = node_relative->composition_g * local_g;
    _node->b = node_relative->composition_b * local_b;
}
//////////////////////////////////////////////////////////////////////////
static uint32_t __get_movie_composition_data_node_count( const aeMovieCompositionData * _compositionData )
{
	uint32_t count = _compositionData->layer_count;
		
	const aeMovieLayerData * it_layer = _compositionData->layers;
	const aeMovieLayerData * it_layer_end = _compositionData->layers + _compositionData->layer_count;
	for( ; it_layer != it_layer_end; ++it_layer )
	{
		const aeMovieLayerData * layer = it_layer;

		uint8_t layer_type = layer->type;

		switch( layer_type )
		{
		case AE_MOVIE_LAYER_TYPE_MOVIE:
		case AE_MOVIE_LAYER_TYPE_SUB_MOVIE:
			{
				uint32_t movie_layer_count = __get_movie_composition_data_node_count( layer->sub_composition );

				count += movie_layer_count;
			}break;
		default:
			{
			}break;
		}
	}

	return count;
}
//////////////////////////////////////////////////////////////////////////
static ae_bool_t __setup_movie_node_track_matte( aeMovieNode * _nodes, uint32_t * _iterator, const aeMovieCompositionData * _compositionData, aeMovieNode * _trackMatte )
{
	const aeMovieLayerData * it_layer = _compositionData->layers;
	const aeMovieLayerData * it_layer_end = _compositionData->layers + _compositionData->layer_count;
	for( ; it_layer != it_layer_end; ++it_layer )
	{
		const aeMovieLayerData * layer = it_layer;

		aeMovieNode * node = _nodes + ((*_iterator)++);

		uint8_t layer_type = node->layer->type;

		if( _trackMatte == AE_NULL )
		{
			if( layer->has_track_matte == AE_TRUE )
			{
				switch( layer_type )
				{
				case AE_MOVIE_LAYER_TYPE_MOVIE:
				case AE_MOVIE_LAYER_TYPE_SUB_MOVIE:
					{
						uint32_t sub_composition_node_count = __get_movie_composition_data_node_count( node->layer->sub_composition );

						aeMovieNode * track_matte_node = _nodes + (*_iterator) + sub_composition_node_count;

						node->track_matte = track_matte_node;
					}break;
				default:
					{
						aeMovieNode * track_matte_node = _nodes + (*_iterator);

						node->track_matte = track_matte_node;
					}break;
				}
			}
			else
			{
				node->track_matte = AE_NULL;
				node->track_matte_data = AE_NULL;
			}
		}
		else
		{
			if( layer->has_track_matte == AE_TRUE )
			{
				return AE_FALSE;
			}
			else
			{
				node->track_matte = _trackMatte;
			}
		}

		switch( layer_type )
		{
		case AE_MOVIE_LAYER_TYPE_MOVIE:
		case AE_MOVIE_LAYER_TYPE_SUB_MOVIE:
			{
				if( __setup_movie_node_track_matte( _nodes, _iterator, layer->sub_composition, node->track_matte ) == AE_FALSE )
				{
					return AE_FALSE;
				}
			}break;
		default:
			{
			}break;
		}
	}

	return AE_TRUE;
}
//////////////////////////////////////////////////////////////////////////
static void __setup_movie_node_relative( aeMovieNode * _nodes, uint32_t * _iterator, const aeMovieCompositionData * _compositionData, aeMovieNode * _parent )
{
	uint32_t begin_index = *_iterator;

	const aeMovieLayerData *it_layer = _compositionData->layers;
	const aeMovieLayerData *it_layer_end = _compositionData->layers + _compositionData->layer_count;
	for( ; it_layer != it_layer_end; ++it_layer )
	{
		const aeMovieLayerData * layer = it_layer;

		aeMovieNode * node = _nodes + ((*_iterator)++);

		node->layer = layer;
		node->matrix_revision = 0;

		node->active = AE_FALSE;
		node->ignore = AE_FALSE;
		node->animate = AE_MOVIE_NODE_ANIMATE_STATIC;

		if( layer->parent_index == 0 )
		{
			node->relative = _parent;
		}

		node->current_time = 0.f;

		switch( layer->type )
		{
		case AE_MOVIE_LAYER_TYPE_MOVIE:
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
    	
	const aeMovieLayerData *it_layer2 = _compositionData->layers;
	const aeMovieLayerData *it_layer2_end = _compositionData->layers + _compositionData->layer_count;
	for( ; it_layer2 != it_layer2_end; ++it_layer2 )
	{
		const aeMovieLayerData * layer = it_layer2;

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
static void __setup_movie_node_time( aeMovieNode * _nodes, uint32_t * _iterator, const aeMovieCompositionData * _compositionData, aeMovieNode * _parent, float _stretch, float _startTime )
{
	
	const aeMovieLayerData *it_layer = _compositionData->layers;
	const aeMovieLayerData *it_layer_end = _compositionData->layers + _compositionData->layer_count;
	for( ; it_layer != it_layer_end; ++it_layer )
	{
		const aeMovieLayerData * layer = it_layer;

		aeMovieNode * node = _nodes + ((*_iterator)++);

		if( _parent == AE_NULL )
		{
			node->start_time = 0.f;
			node->in_time = layer->in_time;
			node->out_time = layer->out_time;
		}
		else
		{
			//node->start_time = _startTime;

			float layer_in = _parent->in_time + layer->in_time * _stretch - _startTime;
			float parent_in = _parent->in_time;

			if( parent_in > layer_in )
			{
				node->start_time = parent_in - layer_in;
				node->in_time = parent_in;
			}
			else
			{
				node->start_time = 0.f;
				node->in_time = layer_in;
			}

			//node->in_time = max_f_f( layer_in, parent_in );

			float layer_out = _parent->in_time + layer->out_time * _stretch - _startTime;
			float parent_out = _parent->out_time;

			node->out_time = min_f_f( layer_out, parent_out );
            
            if( node->out_time <= 0.f || node->out_time < node->in_time )
            {
                node->in_time = 0.f;
                node->out_time = 0.f;
                node->ignore = AE_TRUE;
            }
        }

		node->stretch = _stretch;

		switch( layer->type )
		{
		case AE_MOVIE_LAYER_TYPE_MOVIE:
		case AE_MOVIE_LAYER_TYPE_SUB_MOVIE:
			{
				float to_stretch = _stretch * layer->stretch;
				float to_startTime = _startTime + layer->start_time;

				__setup_movie_node_time( _nodes, _iterator, layer->sub_composition, node, to_stretch, to_startTime );
			}break;
		default:
			{
			}break;
		}
	}
}
//////////////////////////////////////////////////////////////////////////
static void __setup_movie_node_blend_mode( aeMovieNode * _nodes, uint32_t * _iterator, const aeMovieCompositionData * _compositionData, aeMovieNode * _parent, aeMovieBlendMode _blendMode )
{	
	const aeMovieLayerData *it_layer = _compositionData->layers;
	const aeMovieLayerData *it_layer_end = _compositionData->layers + _compositionData->layer_count;
	for( ; it_layer != it_layer_end; ++it_layer )
	{
		const aeMovieLayerData * layer = it_layer;

		aeMovieNode * node = _nodes + ((*_iterator)++);

		if( _blendMode != AE_MOVIE_BLEND_NORMAL )
		{
			node->blend_mode = _blendMode;
		}
		else
		{
			node->blend_mode = layer->blend_mode;
		}

		aeMovieBlendMode composition_blend_mode = AE_MOVIE_BLEND_NORMAL;

		if( layer->sub_composition != AE_NULL )
		{
			composition_blend_mode = node->blend_mode;
		}

		switch( layer->type )
		{
		case AE_MOVIE_LAYER_TYPE_MOVIE:
		case AE_MOVIE_LAYER_TYPE_SUB_MOVIE:
			{
				__setup_movie_node_blend_mode( _nodes, _iterator, layer->sub_composition, node, composition_blend_mode );
			}break;
		default:
			{
			}break;
		}
	}
}
//////////////////////////////////////////////////////////////////////////
static void __setup_movie_node_camera( aeMovieComposition * _composition, uint32_t * _iterator, const aeMovieCompositionData * _compositionData, aeMovieNode * _parent, const void * _cameraData )
{	
	const aeMovieLayerData *it_layer = _compositionData->layers;
	const aeMovieLayerData *it_layer_end = _compositionData->layers + _compositionData->layer_count;
	for( ; it_layer != it_layer_end; ++it_layer )
	{
		const aeMovieLayerData * layer = it_layer;

		aeMovieNode * node = _composition->nodes + ((*_iterator)++);

		if( _compositionData->has_threeD == AE_TRUE )
		{
			float width = _compositionData->width;
			float height = _compositionData->height;
			float zoom = _compositionData->cameraZoom;

			ae_vector3_t camera_position;
			camera_position[0] = width * 0.5f;
			camera_position[1] = height * 0.5f;
			camera_position[2] = -zoom;

			ae_vector3_t camera_direction;
			camera_direction[0] = 0.f;
			camera_direction[1] = 0.f;
			camera_direction[2] = 1.f;

			float camera_fov = make_camera_fov( height, zoom );

			node->camera_data = (*_composition->providers.camera_provider)(_compositionData->name, camera_position, camera_direction, camera_fov, width, height, _composition->provider_data);
		}
		else
		{
			node->camera_data = _cameraData;
		}

		switch( layer->type )
		{
		case AE_MOVIE_LAYER_TYPE_SUB_MOVIE:
		case AE_MOVIE_LAYER_TYPE_MOVIE:
			{
				__setup_movie_node_camera( _composition, _iterator, layer->sub_composition, node, node->camera_data );
			}break;
		default:
			{
			}break;
		}
	}
}
//////////////////////////////////////////////////////////////////////////
static void __setup_movie_node_matrix( aeMovieComposition * _composition )
{
	_composition->update_revision++;
	uint32_t update_revision = _composition->update_revision;
	
	aeMovieNode *it_node = _composition->nodes;
	aeMovieNode *it_node_end = _composition->nodes + _composition->node_count;
	for( ; it_node != it_node_end; ++it_node )
	{
		aeMovieNode * node = it_node;

		__update_movie_composition_node_matrix( _composition, node, update_revision, 0, AE_FALSE, 0.f );
	}
}
//////////////////////////////////////////////////////////////////////////
static void __setup_movie_composition_element( aeMovieComposition * _composition )
{
	const aeMovieCompositionData * compositionData = _composition->composition_data;
	
	aeMovieNode* it_node = _composition->nodes;
	aeMovieNode* it_node_end = _composition->nodes + _composition->node_count;
	for( ; it_node != it_node_end; ++it_node )
	{
		aeMovieNode * node = it_node;

		uint8_t type = node->layer->type;

		void * element_data = (*_composition->providers.node_provider)(node->layer, node->layer->resource, node->matrix, _composition->provider_data);

		node->element_data = element_data;
	}
}
//////////////////////////////////////////////////////////////////////////
static void __setup_movie_composition_active( aeMovieComposition * _composition )
{
	const aeMovieCompositionData * compositionData = _composition->composition_data;

	
	aeMovieNode *it_node = _composition->nodes;
	aeMovieNode *it_node_end = _composition->nodes + _composition->node_count;
	for( ; it_node != it_node_end; ++it_node )
	{
		aeMovieNode * node = it_node;

		if( node->ignore == AE_TRUE )
		{
			continue;
		}

		if( equal_f_z( node->in_time ) == AE_TRUE )
		{
			node->active = AE_TRUE;
		}
	}
}
//////////////////////////////////////////////////////////////////////////
static void * __dummy_ae_movie_composition_node_camera( const ae_char_t * _name, const ae_vector3_t _position, const ae_vector3_t _direction, float _fov, float _width, float _height, void * _data )
{
	(void)_name;
	(void)_position;
	(void)_direction;
	(void)_fov;
	(void)_width;
	(void)_height;
	(void)_data;

	return AE_NULL;
}
//////////////////////////////////////////////////////////////////////////
static void * __dummy_ae_movie_composition_node_provider( const aeMovieLayerData * _layerData, const aeMovieResource * _resource, const ae_matrix4_t _matrix, void * _data )
{
	(void)_layerData;
	(void)_resource;
	(void)_data;
	(void)_matrix;

	return AE_NULL;
}
//////////////////////////////////////////////////////////////////////////
static void __dummy_ae_movie_composition_node_destroyer( const void * _element, uint32_t _type, void * _data )
{
	(void)_element;
	(void)_type;
	(void)_data;
}
//////////////////////////////////////////////////////////////////////////
static void __dummy_ae_movie_composition_node_update( const void * _element, uint32_t _type, ae_bool_t _loop, aeMovieNodeUpdateState _state, float _offset, const ae_matrix4_t _matrix, float _opacity, void * _data )
{
	(void)_element;
	(void)_type;
	(void)_loop;
	(void)_state;
	(void)_offset;
	(void)_matrix;
	(void)_opacity;
	(void)_data;
}
//////////////////////////////////////////////////////////////////////////
static void * __dummy_ae_movie_composition_track_matte_update( const void * _element, uint32_t _type, ae_bool_t _loop, aeMovieNodeUpdateState _state, float _offset, const aeMovieRenderMesh * _mesh, void * _track_matte_data, void * _data )
{
	(void)_element;
	(void)_type;
	(void)_loop;
	(void)_state;
	(void)_offset;
	(void)_mesh;
	(void)_track_matte_data;
	(void)_data;

	return AE_NULL;
}
//////////////////////////////////////////////////////////////////////////
static void __dummy_ae_movie_node_event( const void * _element, const ae_char_t * _name, const ae_matrix4_t _matrix, float _opacity, ae_bool_t _begin, void * _data )
{
	(void)_element;
	(void)_name;
	(void)_matrix;
	(void)_opacity;
	(void)_data;
}
//////////////////////////////////////////////////////////////////////////
static void __dummy_ae_movie_composition_state( aeMovieCompositionStateFlag _state, void * _data )
{
	(void)_state;
	(void)_data;
}
//////////////////////////////////////////////////////////////////////////
aeMovieComposition * ae_create_movie_composition( const aeMovieData * _movieData, const aeMovieCompositionData * _compositionData, const aeMovieCompositionProviders * providers, void * _data )
{
	aeMovieComposition * composition = NEW( _movieData->instance, aeMovieComposition );

	composition->movie_data = _movieData;
	composition->composition_data = _compositionData;

	composition->update_revision = 0;
	composition->time = 0.f;
	composition->work_area_begin = 0.f;
	composition->work_area_end = _compositionData->duration;
	composition->loop = AE_FALSE;
	composition->interpolate = AE_TRUE;

	composition->play = AE_FALSE;
	composition->interrupt = AE_FALSE;

	uint32_t node_count = __get_movie_composition_data_node_count( _compositionData );

	composition->node_count = node_count;
	composition->nodes = NEWN( _movieData->instance, aeMovieNode, node_count );

	composition->providers.camera_provider = providers->camera_provider ? providers->camera_provider : &__dummy_ae_movie_composition_node_camera;
	composition->providers.node_provider = providers->node_provider ? providers->node_provider : &__dummy_ae_movie_composition_node_provider;
	composition->providers.node_destroyer = providers->node_destroyer ? providers->node_destroyer : &__dummy_ae_movie_composition_node_destroyer;
	composition->providers.node_update = providers->node_update ? providers->node_update : &__dummy_ae_movie_composition_node_update;
	composition->providers.track_matte_update = providers->track_matte_update ? providers->track_matte_update : &__dummy_ae_movie_composition_track_matte_update;
	composition->providers.event = providers->event ? providers->event : &__dummy_ae_movie_node_event;
	composition->providers.composition_state = providers->composition_state ? providers->composition_state : &__dummy_ae_movie_composition_state;

	composition->provider_data = _data;

	uint32_t node_relative_iterator = 0;

	__setup_movie_node_relative( composition->nodes, &node_relative_iterator, _compositionData, AE_NULL );

	uint32_t node_track_matte_iterator = 0;

	if( __setup_movie_node_track_matte( composition->nodes, &node_track_matte_iterator, _compositionData, AE_NULL ) == AE_FALSE )
	{
		return AE_NULL;
	}

	uint32_t node_time_iterator = 0;

	__setup_movie_node_time( composition->nodes, &node_time_iterator, _compositionData, AE_NULL, 1.f, 0.f );

	uint32_t node_blend_mode_iterator = 0;

	__setup_movie_node_blend_mode( composition->nodes, &node_blend_mode_iterator, _compositionData, AE_NULL, AE_MOVIE_BLEND_NORMAL );

	uint32_t node_camera_iterator = 0;

	__setup_movie_node_camera( composition, &node_camera_iterator, composition->composition_data, AE_NULL, AE_NULL );

	__setup_movie_node_matrix( composition );

	__setup_movie_composition_element( composition );
	__setup_movie_composition_active( composition );

	return composition;
}
//////////////////////////////////////////////////////////////////////////
void ae_destroy_movie_composition( const aeMovieComposition * _composition )
{	
	const aeMovieNode *it_node = _composition->nodes;
	const aeMovieNode *it_node_end = _composition->nodes + _composition->node_count;
	for( ; it_node != it_node_end; ++it_node )
	{
		const aeMovieNode * node = it_node;

		if( node->animate != AE_MOVIE_NODE_ANIMATE_STATIC && node->animate != AE_MOVIE_NODE_ANIMATE_END )
		{
			(*_composition->providers.track_matte_update)(node->element_data, node->layer->type, AE_FALSE, AE_MOVIE_NODE_UPDATE_END, 0.f, AE_NULL, node->track_matte_data, _composition->provider_data);			
		}

		(*_composition->providers.node_destroyer)(node->element_data, node->layer->type, _composition->provider_data);
	}

	const aeMovieInstance * instance = _composition->movie_data->instance;

	DELETE( instance, _composition->nodes );

	DELETE( instance, _composition );
}
//////////////////////////////////////////////////////////////////////////
ae_bool_t ae_get_movie_composition_anchor_point( const aeMovieComposition * _composition, ae_vector3_t _point )
{
	if( _composition->composition_data->flags & AE_MOVIE_COMPOSITION_ANCHOR_POINT )
	{
		_point[0] = _composition->composition_data->anchor_point[0];
		_point[1] = _composition->composition_data->anchor_point[1];
		_point[2] = _composition->composition_data->anchor_point[2];

		return AE_TRUE;
	}

	return AE_FALSE;
}
//////////////////////////////////////////////////////////////////////////
uint32_t ae_get_movie_composition_max_render_node( const aeMovieComposition * _composition )
{
	uint32_t max_render_node = 0;

	uint32_t node_count = _composition->node_count;

	uint32_t iterator = 0;
	for( ; iterator != node_count; ++iterator )
	{
		const aeMovieNode * node = _composition->nodes + iterator;

		const aeMovieLayerData * layer = node->layer;

		if( layer->renderable == AE_FALSE )
		{
			continue;
		}

		++max_render_node;
	}

	return max_render_node;
}
//////////////////////////////////////////////////////////////////////////
void ae_set_movie_composition_loop( aeMovieComposition * _composition, ae_bool_t _loop )
{
	_composition->loop = _loop;
}
//////////////////////////////////////////////////////////////////////////
void ae_set_movie_composition_interpolate( aeMovieComposition * _composition, ae_bool_t _interpolate )
{
	_composition->interpolate = _interpolate;
}
//////////////////////////////////////////////////////////////////////////
ae_bool_t ae_set_movie_composition_work_area( aeMovieComposition * _composition, float _begin, float _end )
{
	float duration = _composition->composition_data->duration;

	if( _begin < 0.f || _end < 0.f || _begin > duration || _end > duration || _begin > _end )
	{
		return AE_FALSE;
	}

	_composition->work_area_begin = _begin;
	_composition->work_area_end = _end;

	ae_set_movie_composition_time( _composition, _begin );

	return AE_TRUE;
}
//////////////////////////////////////////////////////////////////////////
void ae_remove_movie_composition_work_area( aeMovieComposition * _composition )
{
	_composition->work_area_begin = 0.f;
	_composition->work_area_end = _composition->composition_data->duration;

	ae_set_movie_composition_time( _composition, 0.f );
}
//////////////////////////////////////////////////////////////////////////
void ae_play_movie_composition( aeMovieComposition * _composition, float _time )
{
	if( _composition->play == AE_TRUE )
	{
		return;
	}

	if( _time >= 0.f )
	{
		float work_time = minimax_f_f( _time, _composition->work_area_begin, _composition->work_area_end );

		ae_set_movie_composition_time( _composition, work_time );
	}

	_composition->play = AE_TRUE;
	_composition->interrupt = AE_FALSE;

	(_composition->providers.composition_state)(AE_MOVIE_COMPOSITION_PLAY, _composition->provider_data);
}
//////////////////////////////////////////////////////////////////////////
void ae_stop_movie_composition( aeMovieComposition * _composition )
{
	if( _composition->play == AE_FALSE )
	{
		return;
	}

	aeMovieNode *it_node = _composition->nodes;
	aeMovieNode *it_node_end = _composition->nodes + _composition->node_count;
	for( ; it_node != it_node_end; ++it_node )
	{
		aeMovieNode * node = it_node;

		if( node->animate != AE_MOVIE_NODE_ANIMATE_STATIC && node->animate != AE_MOVIE_NODE_ANIMATE_END )
		{
			(*_composition->providers.track_matte_update)(node->element_data, node->layer->type, AE_FALSE, AE_MOVIE_NODE_UPDATE_END, 0.f, AE_NULL, node->track_matte_data, _composition->provider_data);

			node->animate = AE_MOVIE_NODE_ANIMATE_STATIC;
			node->track_matte_data = AE_NULL;
		}
	}

	_composition->play = AE_FALSE;
	_composition->interrupt = AE_FALSE;

	(_composition->providers.composition_state)(AE_MOVIE_COMPOSITION_STOP, _composition->provider_data);
}
//////////////////////////////////////////////////////////////////////////
static float __get_movie_loop_work_begin( const aeMovieComposition * _composition )
{
	ae_bool_t loop = _composition->loop;

	if( loop == AE_TRUE )
	{
		float work_begin = max_f_f( _composition->composition_data->loop_segment[0], _composition->work_area_begin );

		return work_begin;
	}
	else
	{
		float work_begin = _composition->work_area_begin;

		return work_begin;
	}
}
//////////////////////////////////////////////////////////////////////////
static float __get_movie_loop_work_end( const aeMovieComposition * _composition )
{
	ae_bool_t loop = _composition->loop;

	if( loop == AE_TRUE )
	{
		float work_end = min_f_f( _composition->composition_data->loop_segment[1], _composition->work_area_end );

		return work_end;
	}
	else
	{
		float work_end = _composition->work_area_end;

		return work_end;
	}
}
//////////////////////////////////////////////////////////////////////////
void ae_interrupt_movie_composition( aeMovieComposition * _composition, ae_bool_t _skip )
{
	if( _composition->play == AE_FALSE )
	{
		return;
	}

	if( _composition->loop == AE_FALSE )
	{
		return;
	}

	_composition->interrupt = AE_TRUE;

	if( _skip == AE_TRUE )
	{
		float work_end = __get_movie_loop_work_end( _composition );

		ae_set_movie_composition_time( _composition, work_end );
	}

	(_composition->providers.composition_state)(AE_MOVIE_COMPOSITION_INTERRUPT, _composition->provider_data);
}
//////////////////////////////////////////////////////////////////////////
static void __update_movie_composition_node_state( aeMovieComposition * _composition, aeMovieNode * _node, ae_bool_t _loop, ae_bool_t _begin, float _time )
{
	if( _node->element_data == AE_NULL )
	{
		return;
	}

	if( _begin == AE_TRUE )
	{
		if( _node->animate == AE_MOVIE_NODE_ANIMATE_STATIC || _node->animate == AE_MOVIE_NODE_ANIMATE_END )
		{
			_node->animate = AE_MOVIE_NODE_ANIMATE_BEGIN;

			(*_composition->providers.node_update)(_node->element_data, _node->layer->type, _loop, AE_MOVIE_NODE_UPDATE_BEGIN, _node->start_time + _time - _node->in_time, _node->matrix, _node->opacity, _composition->provider_data);
		}
		else
		{
			_node->animate = AE_MOVIE_NODE_ANIMATE_PROCESS;

			(*_composition->providers.node_update)(_node->element_data, _node->layer->type, _loop, AE_MOVIE_NODE_UPDATE_UPDATE, 0.f, _node->matrix, _node->opacity, _composition->provider_data);
		}
	}
	else
	{
		if( _node->animate == AE_MOVIE_NODE_ANIMATE_PROCESS || _node->animate == AE_MOVIE_NODE_ANIMATE_BEGIN )
		{
			_node->animate = AE_MOVIE_NODE_ANIMATE_END;

			(*_composition->providers.node_update)(_node->element_data, _node->layer->type, _loop, AE_MOVIE_NODE_UPDATE_END, 0.f, _node->matrix, _node->opacity, _composition->provider_data);
		}
		else
		{
			_node->animate = AE_MOVIE_NODE_ANIMATE_STATIC;
		}
	}
}
//////////////////////////////////////////////////////////////////////////
static void __make_mesh_vertices( const aeMovieMesh * _mesh, const ae_matrix4_t _matrix, aeMovieRenderMesh * _render )
{
	_render->vertexCount = _mesh->vertex_count;
	_render->indexCount = _mesh->indices_count;

	uint32_t vertex_count = _mesh->vertex_count;

	uint32_t i = 0;
	for( ; i != vertex_count; ++i )
	{
		mul_v3_v2_m4( _render->position[i], _mesh->positions[i], _matrix );
	}

	_render->uv = _mesh->uvs;
	_render->indices = _mesh->indices;
}
//////////////////////////////////////////////////////////////////////////
static void __make_layer_sprite_vertices( const aeMovieInstance * _instance, float _offset_x, float _offset_y, float _width, float _height, const ae_matrix4_t _matrix, aeMovieRenderMesh * _render )
{
    ae_vector2_t v_position[4];

	float * v = &v_position[0][0];

	*v++ = _offset_x + _width * 0.f;
	*v++ = _offset_y + _height * 0.f;
	*v++ = _offset_x + _width * 1.f;
	*v++ = _offset_y + _height * 0.f;
	*v++ = _offset_x + _width * 1.f;
	*v++ = _offset_y + _height * 1.f;
	*v++ = _offset_x + _width * 0.f;
	*v++ = _offset_y + _height * 1.f;

	_render->vertexCount = 4;
	_render->indexCount = 6;

	mul_v3_v2_m4( _render->position[0], v_position[0], _matrix );
	mul_v3_v2_m4( _render->position[1], v_position[1], _matrix );
	mul_v3_v2_m4( _render->position[2], v_position[2], _matrix );
	mul_v3_v2_m4( _render->position[3], v_position[3], _matrix );

	_render->uv = _instance->sprite_uv;
	_render->indices = _instance->sprite_indices;
}
//////////////////////////////////////////////////////////////////////////
static void __make_layer_mesh_vertices( const aeMovieLayerMesh * _layerMesh, uint32_t _frame, const ae_matrix4_t _matrix, aeMovieRenderMesh * _render )
{
	const aeMovieMesh * mesh = (_layerMesh->immutable == AE_TRUE) ? &_layerMesh->immutable_mesh : (_layerMesh->meshes + _frame);

	__make_mesh_vertices( mesh, _matrix, _render );
}
//////////////////////////////////////////////////////////////////////////
static float __bezier_point( float a, float b, float c, float d, float t )
{
	float t2 = t * t;
	float t3 = t2 * t;

	float ti = 1.f - t;
	float ti2 = ti * ti;
	float ti3 = ti2 * ti;

	return a * ti3 + 3.f * b * t * ti2 + 3.f * c * t2 * ti + d * t3;
}
//////////////////////////////////////////////////////////////////////////
static float __bezier_warp_x( const aeMovieBezierWarp * _bezier_warp, float _u, float _v )
{
	const ae_vector2_t * c = _bezier_warp->corners;
	const ae_vector2_t * b = _bezier_warp->beziers;

	float bx = __bezier_point( c[0][0], b[0][0], b[7][0], c[3][0], _v );
	float ex = __bezier_point( c[1][0], b[3][0], b[4][0], c[2][0], _v );

	float mx0 = (b[1][0] - b[6][0]) * (1.f - _v) + b[6][0];
	float mx1 = (b[2][0] - b[5][0]) * (1.f - _v) + b[5][0];

	float x = __bezier_point( bx, mx0, mx1, ex, _u );

	return x;
}
//////////////////////////////////////////////////////////////////////////
static float __bezier_warp_y( const aeMovieBezierWarp * _bezier_warp, float _u, float _v )
{
	const ae_vector2_t * c = _bezier_warp->corners;
	const ae_vector2_t * b = _bezier_warp->beziers;

	float by = __bezier_point( c[0][1], b[0][1], b[7][1], c[3][1], _v );
	float ey = __bezier_point( c[1][1], b[3][1], b[4][1], c[2][1], _v );

	float my0 = (b[1][1] - b[6][1]) * (1.f - _v) + b[6][1];
	float my1 = (b[2][1] - b[5][1]) * (1.f - _v) + b[5][1];

	float y = __bezier_point( by, my0, my1, ey, _u );

	return y;
}
//////////////////////////////////////////////////////////////////////////
static void __make_bezier_warp_vertices( const aeMovieInstance * _instance, const aeMovieBezierWarp * _bezierWarp, const ae_matrix4_t _matrix, aeMovieRenderMesh * _render )
{
	_render->vertexCount = AE_MOVIE_BEZIER_WARP_GRID_VERTEX_COUNT;
	_render->indexCount = AE_MOVIE_BEZIER_WARP_GRID_INDICES_COUNT;

	float du = 0.f;
	float dv = 0.f;

	ae_vector3_t * positions = _render->position;

	uint32_t v = 0;
	for( ; v != AE_MOVIE_BEZIER_WARP_GRID; ++v )
	{
		uint32_t u = 0;
		for( ; u != AE_MOVIE_BEZIER_WARP_GRID; ++u )
		{
			float x = __bezier_warp_x( _bezierWarp, du, dv );
			float y = __bezier_warp_y( _bezierWarp, du, dv );

			float position[2] = {x, y};

			mul_v3_v2_m4( *positions++, position, _matrix );

			du += ae_movie_bezier_warp_grid_invf;
		}

		du = 0.f;
		dv += ae_movie_bezier_warp_grid_invf;
	}

	_render->uv = _instance->bezier_warp_uv;
	_render->indices = _instance->bezier_warp_indices;
}
//////////////////////////////////////////////////////////////////////////
static void __make_layer_bezier_warp_vertices( const aeMovieInstance * _instance, const aeMovieLayerBezierWarp * _bezierWarp, uint32_t _frame, float _t, const ae_matrix4_t _matrix, aeMovieRenderMesh * _render )
{
	if( _bezierWarp->immutable == AE_TRUE )
	{
		__make_bezier_warp_vertices( _instance, &_bezierWarp->immutable_bezier_warp, _matrix, _render );
	}
	else
	{
		aeMovieBezierWarp bezierWarp;

		const aeMovieBezierWarp * bezier_warp_frame_current = _bezierWarp->bezier_warps + _frame + 0;
		const aeMovieBezierWarp * bezier_warp_frame_next = _bezierWarp->bezier_warps + _frame + 1;

        const ae_vector2_t * current_corners = bezier_warp_frame_current->corners;
        const ae_vector2_t * next_corners = bezier_warp_frame_next->corners;

		linerp_f2( bezierWarp.corners[0], current_corners[0], next_corners[0], _t );
		linerp_f2( bezierWarp.corners[1], current_corners[1], next_corners[1], _t );
		linerp_f2( bezierWarp.corners[2], current_corners[2], next_corners[2], _t );
		linerp_f2( bezierWarp.corners[3], current_corners[3], next_corners[3], _t );

        const ae_vector2_t * current_beziers = bezier_warp_frame_current->beziers;
        const ae_vector2_t * next_beziers = bezier_warp_frame_next->beziers;

		linerp_f2( bezierWarp.beziers[0], current_beziers[0], next_beziers[0], _t );
		linerp_f2( bezierWarp.beziers[1], current_beziers[1], next_beziers[1], _t );
		linerp_f2( bezierWarp.beziers[2], current_beziers[2], next_beziers[2], _t );
		linerp_f2( bezierWarp.beziers[3], current_beziers[3], next_beziers[3], _t );
		linerp_f2( bezierWarp.beziers[4], current_beziers[4], next_beziers[4], _t );
		linerp_f2( bezierWarp.beziers[5], current_beziers[5], next_beziers[5], _t );
		linerp_f2( bezierWarp.beziers[6], current_beziers[6], next_beziers[6], _t );
		linerp_f2( bezierWarp.beziers[7], current_beziers[7], next_beziers[7], _t );

		__make_bezier_warp_vertices( _instance, &bezierWarp, _matrix, _render );
	}
}
//////////////////////////////////////////////////////////////////////////
static uint32_t __compute_movie_node_frame( const aeMovieNode * _node, ae_bool_t _interpolate, float * _t )
{
    const aeMovieLayerData * layer = _node->layer;

    float frameDurationInv = layer->composition->frameDurationInv;

    uint32_t frame;

    if( layer->reverse_time == AE_TRUE )
    {
        float frame_time = (_node->out_time - _node->in_time - _node->current_time) * frameDurationInv;

        frame = (uint32_t)frame_time;

        if( _interpolate == AE_TRUE )
        {
            *_t = frame_time - (float)frame;
        }
    }
    else
    {
        float frame_time = (_node->current_time) * frameDurationInv;

        frame = (uint32_t)frame_time;

        if( _interpolate == AE_TRUE )
        {
            *_t = frame_time - (float)frame;
        }
    }

    return frame;
}
//////////////////////////////////////////////////////////////////////////
static void __compute_movie_node( const aeMovieComposition * _composition, const aeMovieNode * _node, aeMovieRenderMesh * _render, ae_bool_t _trackmatte )
{
	const aeMovieInstance * instance = _composition->movie_data->instance;
	const aeMovieLayerData * layer = _node->layer;
	const aeMovieResource * resource = layer->resource;

	ae_bool_t interpolate = _composition->interpolate;

	uint8_t layer_type = layer->type;

	_render->layer_type = layer_type;

	_render->blend_mode = _node->blend_mode;

	if( resource != AE_NULL )
	{
		_render->resource_type = resource->type;
		_render->resource_data = resource->data;
	}
	else
	{
		_render->resource_type = AE_MOVIE_RESOURCE_NONE;
		_render->resource_data = AE_NULL;
	}

	_render->camera_data = _node->camera_data;
	_render->element_data = _node->element_data;

	if( _node->track_matte != AE_NULL && _node->track_matte->active == AE_TRUE )
	{
		_render->track_matte_data = _node->track_matte->track_matte_data;
	}
	else
	{
		_render->track_matte_data = AE_NULL;
	}

	float t_frame = 0.f;
	uint32_t frame = __compute_movie_node_frame( _node, interpolate, &t_frame );

	switch( layer_type )
	{
	case AE_MOVIE_LAYER_TYPE_SHAPE:
		{
            __make_layer_mesh_vertices(layer->mesh, frame, _node->matrix, _render);

			_render->r = _node->r;
			_render->g = _node->g;
			_render->b = _node->b;
			_render->a = _node->opacity;
		}break;
	case AE_MOVIE_LAYER_TYPE_SOLID:
		{
			aeMovieResourceSolid * resource_solid = (aeMovieResourceSolid *)resource;

			if( layer->mesh != AE_NULL )
			{
				__make_layer_mesh_vertices( layer->mesh, frame, _node->matrix, _render );
			}
			else if( layer->bezier_warp != AE_NULL )
			{
				__make_layer_bezier_warp_vertices( instance, layer->bezier_warp, frame, t_frame, _node->matrix, _render );
			}
			else
			{
				float width = resource_solid->width;
				float height = resource_solid->height;

				__make_layer_sprite_vertices( instance, 0.f, 0.f, width, height, _node->matrix, _render);
			}

			_render->r = _node->r * resource_solid->r;
			_render->g = _node->g * resource_solid->g;
			_render->b = _node->b * resource_solid->b;
			_render->a = _node->opacity;
		}break;
	case AE_MOVIE_LAYER_TYPE_SEQUENCE:
		{
			aeMovieResourceSequence * resource_sequence = (aeMovieResourceSequence *)resource;

			uint32_t frame_sequence;

			if( layer->timeremap != AE_NULL )
			{
				float time = layer->timeremap->times[frame];

				frame_sequence = (uint32_t)(time * resource_sequence->frameDurationInv);
			}
			else
			{
				if( layer->reverse_time == AE_TRUE )
				{
					frame_sequence = (uint32_t)((_node->out_time - _node->in_time - (layer->start_time + _node->current_time)) * resource_sequence->frameDurationInv);
				}
				else
				{
					frame_sequence = (uint32_t)((layer->start_time + _node->current_time) * resource_sequence->frameDurationInv);
				}
			}

			frame_sequence %= resource_sequence->image_count;

			aeMovieResourceImage * resource_image = resource_sequence->images[frame_sequence];

			_render->resource_type = resource_image->type;
			_render->resource_data = resource_image->data;
			
			if( layer->mesh != AE_NULL )
			{
				__make_layer_mesh_vertices( layer->mesh, frame, _node->matrix, _render );
			}
			else if( layer->bezier_warp != AE_NULL )
			{
				__make_layer_bezier_warp_vertices( instance, layer->bezier_warp, frame, t_frame, _node->matrix, _render );
			}
			else if( resource_image->mesh != AE_NULL && _trackmatte == AE_FALSE )
			{
				__make_mesh_vertices( resource_image->mesh, _node->matrix, _render );
			}
			else
			{
				float offset_x = resource_image->offset_x;
				float offset_y = resource_image->offset_y;

				float width = resource_image->trim_width;
				float height = resource_image->trim_height;

				__make_layer_sprite_vertices( instance, offset_x, offset_y, width, height, _node->matrix, _render );
			}

			_render->r = _node->r;
			_render->g = _node->g;
			_render->b = _node->b;
			_render->a = _node->opacity;
		}break;
	case AE_MOVIE_LAYER_TYPE_VIDEO:
		{
			aeMovieResourceVideo * resource_video = (aeMovieResourceVideo *)resource;

			if( layer->mesh != AE_NULL )
			{
				__make_layer_mesh_vertices( layer->mesh, frame, _node->matrix, _render );
			}
			else if( layer->bezier_warp != AE_NULL )
			{
				__make_layer_bezier_warp_vertices( instance, layer->bezier_warp, frame, t_frame, _node->matrix, _render );
			}
			else
			{
				float width = resource_video->width;
				float height = resource_video->height;

				__make_layer_sprite_vertices( instance, 0.f, 0.f, width, height, _node->matrix, _render );
			}

			_render->r = _node->r;
			_render->g = _node->g;
			_render->b = _node->b;
			_render->a = _node->opacity;
		}break;
	case AE_MOVIE_LAYER_TYPE_IMAGE:
		{
			aeMovieResourceImage * resource_image = (aeMovieResourceImage *)resource;

			if( layer->mesh != AE_NULL )
			{
				__make_layer_mesh_vertices( layer->mesh, frame, _node->matrix, _render );
			}
			else if( layer->bezier_warp != AE_NULL )
			{
				__make_layer_bezier_warp_vertices( instance, layer->bezier_warp, frame, t_frame, _node->matrix, _render );
			}
			else if( resource_image->mesh != AE_NULL && _trackmatte == AE_FALSE )
			{
				__make_mesh_vertices( resource_image->mesh, _node->matrix, _render );
			}
			else
			{
				float offset_x = resource_image->offset_x;
				float offset_y = resource_image->offset_y;

				float width = resource_image->trim_width;
				float height = resource_image->trim_height;

				__make_layer_sprite_vertices( instance, offset_x, offset_y, width, height, _node->matrix, _render );
			}

			_render->r = _node->r;
			_render->g = _node->g;
			_render->b = _node->b;
			_render->a = _node->opacity;
		}break;
	default:
		{
			_render->vertexCount = 0;
			_render->indexCount = 0;

			_render->r = _node->r;
			_render->g = _node->g;
			_render->b = _node->b;
			_render->a = _node->opacity;
		}break;
	}
}
//////////////////////////////////////////////////////////////////////////
static void __update_movie_composition_track_matte_state( aeMovieComposition * _composition, aeMovieNode * _node, ae_bool_t _loop, ae_bool_t _begin, float _time )
{
	uint8_t layer_type = _node->layer->type;

	switch( layer_type )
	{
	case AE_MOVIE_LAYER_TYPE_MOVIE:
		{
			return;
		}break;
	default:
		{
		}break;
	}

	aeMovieRenderMesh vertices;
	__compute_movie_node( _composition, _node, &vertices, AE_TRUE );

	if( _begin == AE_TRUE )
	{
		if( _node->animate == AE_MOVIE_NODE_ANIMATE_STATIC || _node->animate == AE_MOVIE_NODE_ANIMATE_END )
		{
			_node->animate = AE_MOVIE_NODE_ANIMATE_BEGIN;

			void * track_matte_data = (*_composition->providers.track_matte_update)(_node->element_data, layer_type, _loop, AE_MOVIE_NODE_UPDATE_BEGIN, _node->start_time + _time - _node->in_time, &vertices, AE_NULL, _composition->provider_data);

			_node->track_matte_data = track_matte_data;
		}
		else
		{
			_node->animate = AE_MOVIE_NODE_ANIMATE_PROCESS;

			(*_composition->providers.track_matte_update)(_node->element_data, layer_type, _loop, AE_MOVIE_NODE_UPDATE_UPDATE, 0.f, &vertices, _node->track_matte_data, _composition->provider_data);
		}
	}
	else
	{
		if( _node->animate == AE_MOVIE_NODE_ANIMATE_PROCESS || _node->animate == AE_MOVIE_NODE_ANIMATE_BEGIN )
		{
			_node->animate = AE_MOVIE_NODE_ANIMATE_END;

			(*_composition->providers.track_matte_update)(_node->element_data, layer_type, _loop, AE_MOVIE_NODE_UPDATE_END, 0.f, &vertices, _node->track_matte_data, _composition->provider_data);

			_node->track_matte_data = AE_NULL;
		}
		else
		{
			_node->animate = AE_MOVIE_NODE_ANIMATE_STATIC;
		}
	}
}
//////////////////////////////////////////////////////////////////////////
static void __update_node( aeMovieComposition * _composition, aeMovieNode * _node, uint32_t _revision, float _time, uint32_t _frameId, float _t, ae_bool_t _loop, ae_bool_t _interpolate, ae_bool_t _begin )
{
#	ifdef AE_MOVIE_DEBUG
	if( __test_error_composition_layer_frame( _composition->movie_data->instance
		, _composition->composition_data
		, _node->layer
		, _frameId
		, "__update_node frame id out count"
		) == AE_FALSE )
	{
		return;
	}
#	endif

	__update_movie_composition_node_matrix( _composition, _node, _revision, _frameId, _interpolate, _t );

	if( _node->layer->timeremap != AE_NULL )
	{

	}

	if( _node->layer->is_track_matte == AE_TRUE )
	{
		__update_movie_composition_track_matte_state( _composition, _node, _loop, _begin, _time );
	}
	else
	{
		__update_movie_composition_node_state( _composition, _node, _loop, _begin, _time );
	}
}
//////////////////////////////////////////////////////////////////////////
static void __update_movie_composition_node( aeMovieComposition * _composition, uint32_t _revision, float _beginTime, float _endTime )
{
	ae_bool_t composition_interpolate = _composition->interpolate;
	ae_bool_t composition_interrupt = _composition->interrupt;
	ae_bool_t composition_loop = _composition->loop;
	
	_composition->time = _endTime;

	float loopBegin = __get_movie_loop_work_begin( _composition );
	float loopEnd = __get_movie_loop_work_end( _composition );

	
	aeMovieNode *it_node = _composition->nodes;
	aeMovieNode *it_node_end = _composition->nodes + _composition->node_count;
	for( ; it_node != it_node_end; ++it_node )
	{
		aeMovieNode * node = it_node;

		if( node->ignore == AE_TRUE )
		{
			continue;
		}

		const aeMovieLayerData * layer = node->layer;

		float frameDurationInv = layer->composition->frameDurationInv;

		float in_time = (_beginTime >= loopBegin && node->in_time <= loopBegin && _endTime >= loopBegin && composition_interrupt == AE_FALSE && composition_loop == AE_TRUE && layer->type != AE_MOVIE_LAYER_TYPE_EVENT) ? loopBegin : node->in_time;
		float out_time = (node->out_time >= loopEnd && composition_interrupt == AE_FALSE && composition_loop == AE_TRUE && layer->type != AE_MOVIE_LAYER_TYPE_EVENT) ? loopEnd : node->out_time;

		uint32_t beginFrame = (uint32_t)(_beginTime * frameDurationInv + 0.001f);
		uint32_t endFrame = (uint32_t)(_endTime * frameDurationInv + 0.001f);
		uint32_t indexIn = (uint32_t)(in_time * frameDurationInv + 0.001f);
		uint32_t indexOut = (uint32_t)(out_time * frameDurationInv + 0.001f);

		if( indexIn > endFrame || indexOut < beginFrame )
		{
			node->active = AE_FALSE;

			continue;
		}

		float current_time = (endFrame >= indexOut) ? out_time - node->in_time + node->start_time : _composition->time - node->in_time + node->start_time;
		float frame_time = current_time / node->stretch * frameDurationInv;

		uint32_t frameId = (uint32_t)frame_time;

		if( node->layer->type == AE_MOVIE_LAYER_TYPE_EVENT )
		{
			node->current_time = current_time;

			__update_movie_composition_node_matrix( _composition, node, _revision, frameId, AE_FALSE, 0.f );

			if( beginFrame < indexIn && endFrame >= indexIn )
			{
				(*_composition->providers.event)(node->element_data, node->layer->name, node->matrix, node->opacity, AE_TRUE, _composition->provider_data);
			}

			if( beginFrame < indexOut && endFrame >= indexOut )
			{
				(*_composition->providers.event)(node->element_data, node->layer->name, node->matrix, node->opacity, AE_FALSE, _composition->provider_data);
			}
		}
		else
		{
			if( indexIn >= beginFrame && indexOut < endFrame )
			{
				node->active = AE_FALSE;

				continue;
			}

			node->current_time = current_time;

			float t = frame_time - (float)frameId;

			ae_bool_t node_loop = ((composition_loop == AE_TRUE && composition_interrupt == AE_FALSE && loopBegin >= node->in_time && node->out_time >= loopEnd && layer->type != AE_MOVIE_LAYER_TYPE_EVENT) || (layer->params & AE_MOVIE_LAYER_PARAM_LOOP)) ? AE_TRUE : AE_FALSE;

			if( beginFrame < indexIn && endFrame >= indexIn && endFrame < indexOut )
			{
				node->active = AE_TRUE;

				ae_bool_t node_interpolate = (node_loop == AE_TRUE) ? composition_interpolate : ((endFrame + 1) < indexOut);

				__update_node( _composition, node, _revision, _endTime, frameId, t, node_loop, node_interpolate, AE_TRUE );
			}
			else if( endFrame >= indexOut && beginFrame >= indexIn && beginFrame < indexOut )
			{
				ae_bool_t node_deactive = (node_loop == AE_TRUE) ? AE_TRUE : AE_FALSE;

				node->active = node_deactive;

				uint32_t frameEnd = indexOut - indexIn;

				__update_node( _composition, node, _revision, _endTime, frameEnd, 0.f, node_loop, AE_FALSE, AE_FALSE );
			}
			else if( beginFrame >= indexIn && endFrame >= indexIn && endFrame < indexOut )
			{
				node->active = AE_TRUE;

				ae_bool_t node_interpolate = composition_interpolate ? ((frameId + 1) < indexOut) : AE_FALSE;

				__update_node( _composition, node, _revision, _endTime, frameId, t, node_loop, node_interpolate, AE_TRUE );
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////
static void __skip_movie_composition_node( aeMovieComposition * _composition, uint32_t _revision, float _beginTime, float _endTime )
{
	ae_bool_t interrupt = _composition->interrupt;
	ae_bool_t loop = _composition->loop;
	float duration = _composition->composition_data->duration;
	float composition_time = _composition->time;

	float loopBegin = 0.f;
	float loopEnd = duration;

	
	aeMovieNode	*it_node = _composition->nodes;
	aeMovieNode *it_node_end = _composition->nodes + _composition->node_count;
	for( ; it_node != it_node_end; ++it_node )
	{
		aeMovieNode * node = it_node;

		if( node->ignore == AE_TRUE )
		{
			continue;
		}

		const aeMovieLayerData * layer = node->layer;

		float frameDurationInv = layer->composition->frameDurationInv;

		float in_time = node->in_time;
		float out_time = node->out_time;

		uint32_t beginFrame = (uint32_t)(_beginTime * frameDurationInv + 0.001f);
		uint32_t endFrame = (uint32_t)(_endTime * frameDurationInv + 0.001f);
		uint32_t indexIn = (uint32_t)(in_time * frameDurationInv + 0.001f);
		uint32_t indexOut = (uint32_t)(out_time * frameDurationInv + 0.001f);

		if( indexIn > endFrame || indexOut < beginFrame )
		{
			node->active = AE_FALSE;

			continue;
		}

		if( indexIn >= beginFrame && indexOut < endFrame )
		{
			node->active = AE_FALSE;

			continue;
		}

		float current_time = (endFrame >= indexOut) ? out_time - node->in_time + node->start_time : _composition->time - node->in_time + node->start_time;
		float frame_time = current_time / node->stretch * frameDurationInv;

		uint32_t frameId = (uint32_t)frame_time;

		node->current_time = current_time;

		if( node->layer->type == AE_MOVIE_LAYER_TYPE_EVENT )
		{
		}
		else
		{
			if( beginFrame < indexIn && endFrame >= indexIn && endFrame < indexOut )
			{
			}
			else if( endFrame >= indexOut && beginFrame >= indexIn && beginFrame < indexOut )
			{
				node->active = AE_FALSE;

				float t = frame_time - (float)frameId;

				__update_node( _composition, node, _revision, _endTime, frameId, t, AE_FALSE, (endFrame + 1) < indexOut, AE_FALSE );
			}
			else if( beginFrame >= indexIn && endFrame >= indexIn && endFrame < indexOut )
			{
			}
		}

		node->animate = AE_MOVIE_NODE_ANIMATE_STATIC;
	}
}
//////////////////////////////////////////////////////////////////////////
void ae_update_movie_composition( aeMovieComposition * _composition, float _timing )
{
	if( _composition->play == AE_FALSE )
	{
		return;
	}

	_composition->update_revision++;
	uint32_t update_revision = _composition->update_revision;

	float prev_time = _composition->time;

	_composition->time += _timing;

	float frameDuration = _composition->composition_data->frameDuration;

	float begin_time = prev_time;

	float duration = _composition->work_area_end - _composition->work_area_begin;

	if( _composition->loop == AE_FALSE || _composition->interrupt == AE_TRUE )
	{
		float last_time = duration - frameDuration;

		if( _composition->time >= last_time )
		{
			__update_movie_composition_node( _composition, update_revision, begin_time, last_time );

			_composition->update_revision++;
			update_revision = _composition->update_revision;

			_composition->play = AE_FALSE;
			_composition->interrupt = AE_FALSE;

			(*_composition->providers.composition_state)(AE_MOVIE_COMPOSITION_END, _composition->provider_data);

			return;
		}
	}
	else
	{
		float loopBegin = max_f_f( _composition->composition_data->loop_segment[0], _composition->work_area_begin );
		float loopEnd = min_f_f( _composition->composition_data->loop_segment[1], _composition->work_area_end );

		float last_time = loopEnd - frameDuration;

		while( _composition->time >= last_time )
		{
			float new_composition_time = _composition->time - last_time + loopBegin;

			__update_movie_composition_node( _composition, update_revision, begin_time, last_time );

			_composition->update_revision++;
			update_revision = _composition->update_revision;

			begin_time = loopBegin;

			_composition->time = new_composition_time;

			(*_composition->providers.composition_state)(AE_MOVIE_COMPOSITION_LOOP_END, _composition->provider_data);
		}
	}

	__update_movie_composition_node( _composition, update_revision, begin_time, _composition->time );
}
//////////////////////////////////////////////////////////////////////////
void ae_set_movie_composition_time( aeMovieComposition * _composition, float _time )
{
	float duration = _composition->composition_data->duration;

	if( _time < 0.f && _time > duration )
	{
		return;
	}

	if( equal_f_f( _composition->time, _time ) == AE_TRUE )
	{
		return;
	}

	_composition->update_revision++;
	uint32_t update_revision = _composition->update_revision;

	if( _composition->time > _time )
	{
		__skip_movie_composition_node( _composition, update_revision, _composition->time, _composition->composition_data->duration );

		_composition->update_revision++;
		update_revision = _composition->update_revision;

		__update_movie_composition_node( _composition, update_revision, 0.f, _time );
	}
	else
	{
		__update_movie_composition_node( _composition, update_revision, _composition->time, _time );
	}
}
//////////////////////////////////////////////////////////////////////////
float ae_get_movie_composition_time( const aeMovieComposition * _composition )
{
	return _composition->time;
}
//////////////////////////////////////////////////////////////////////////
float ae_get_movie_composition_duration( const aeMovieComposition * _composition )
{
	float duration = ae_get_movie_composition_data_duration( _composition->composition_data );

	return duration;
}
//////////////////////////////////////////////////////////////////////////
ae_bool_t ae_set_movie_composition_slot( aeMovieComposition * _composition, const ae_char_t * _slotName, void * _slotData )
{
	const aeMovieInstance * instance = _composition->movie_data->instance;
	
	aeMovieNode *it_node = _composition->nodes;
	aeMovieNode *it_node_end = _composition->nodes + _composition->node_count;
	for( ; it_node != it_node_end; ++it_node )
	{
		aeMovieNode * node = it_node;

		const aeMovieLayerData * layer = node->layer;

		if( layer->type != AE_MOVIE_LAYER_TYPE_SLOT )
		{
			continue;
		}

		if( STRNCMP( instance, layer->name, _slotName, AE_MOVIE_MAX_LAYER_NAME ) != 0 )
		{
			continue;
		}

		node->element_data = _slotData;

		return AE_TRUE;
	}

	return AE_FALSE;
}
//////////////////////////////////////////////////////////////////////////
void * ae_get_movie_composition_slot( const aeMovieComposition * _composition, const ae_char_t * _slotName )
{
	const aeMovieInstance * instance = _composition->movie_data->instance;
	
    const aeMovieNode *it_node = _composition->nodes;
    const aeMovieNode *it_node_end = _composition->nodes + _composition->node_count;
	for( ; it_node != it_node_end; ++it_node )
	{
        const aeMovieNode * node = it_node;

		const aeMovieLayerData * layer = node->layer;

		if( layer->type != AE_MOVIE_LAYER_TYPE_SLOT )
		{
			continue;
		}

		if( STRNCMP( instance, layer->name, _slotName, AE_MOVIE_MAX_LAYER_NAME ) != 0 )
		{
			continue;
		}

		return node->element_data;
	}

	return AE_NULL;
}
//////////////////////////////////////////////////////////////////////////
ae_bool_t ae_has_movie_composition_slot( const aeMovieComposition * _composition, const ae_char_t * _slotName )
{
	const aeMovieInstance * instance = _composition->movie_data->instance;
		
    const aeMovieNode *it_node = _composition->nodes;
    const aeMovieNode *it_node_end = _composition->nodes + _composition->node_count;
	for( ; it_node != it_node_end; ++it_node )
	{
        const aeMovieNode * node = it_node;

		const aeMovieLayerData * layer = node->layer;

		if( layer->type != AE_MOVIE_LAYER_TYPE_SLOT )
		{
			continue;
		}

		if( STRNCMP( instance, layer->name, _slotName, AE_MOVIE_MAX_LAYER_NAME ) != 0 )
		{
			continue;
		}

		return AE_TRUE;
	}

	return AE_FALSE;
}
//////////////////////////////////////////////////////////////////////////
void * ae_remove_movie_composition_slot( aeMovieComposition * _composition, const ae_char_t * _slotName )
{
	const aeMovieInstance * instance = _composition->movie_data->instance;
		
	aeMovieNode *it_node = _composition->nodes;
	aeMovieNode *it_node_end = _composition->nodes + _composition->node_count;
	for( ; it_node != it_node_end; ++it_node )
	{
		aeMovieNode * node = it_node;

		const aeMovieLayerData * layer = node->layer;

		if( layer->type != AE_MOVIE_LAYER_TYPE_SLOT )
		{
			continue;
		}

		if( STRNCMP( instance, layer->name, _slotName, AE_MOVIE_MAX_LAYER_NAME ) != 0 )
		{
			continue;
		}

		void * prev_element_data = node->element_data;

		node->element_data = AE_NULL;

		return prev_element_data;
	}

	return AE_NULL;
}
//////////////////////////////////////////////////////////////////////////
ae_bool_t ae_get_movie_composition_socket(const aeMovieComposition * _composition, const ae_char_t * _slotName, const aeMoviePolygon ** _polygon)
{
    const aeMovieInstance * instance = _composition->movie_data->instance;

    const aeMovieNode *it_node = _composition->nodes;
    const aeMovieNode *it_node_end = _composition->nodes + _composition->node_count;
    for( ; it_node != it_node_end; ++it_node )
    {
        const aeMovieNode * node = it_node;

        const aeMovieLayerData * layer = node->layer;

        if( layer->type != AE_MOVIE_LAYER_TYPE_SOCKET )
        {
            continue;
        }

        if( STRNCMP(instance, layer->name, _slotName, AE_MOVIE_MAX_LAYER_NAME) != 0 )
        {
            continue;
        }

        if( layer->polygon->immutable == AE_TRUE )
        {
            *_polygon = &layer->polygon->immutable_polygon;

            return AE_TRUE;
        }
        else
        {
            uint32_t frame = __compute_movie_node_frame(node, AE_FALSE, AE_NULL);

            *_polygon = layer->polygon->polygons + frame;
        }
    }

    return AE_FALSE;
}
//////////////////////////////////////////////////////////////////////////
ae_bool_t ae_compute_movie_mesh( const aeMovieComposition * _composition, uint32_t * _iterator, aeMovieRenderMesh * _render )
{
	uint32_t render_node_index = *_iterator;
	uint32_t render_node_max_count = _composition->node_count;

	const aeMovieInstance * instance = _composition->movie_data->instance;

	uint32_t iterator = render_node_index;
	for( ; iterator != render_node_max_count; ++iterator )
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

		if( node->track_matte != AE_NULL && node->track_matte->active == AE_FALSE )
		{
			continue;
		}

		*_iterator = iterator + 1;

		__compute_movie_node( _composition, node, _render, AE_FALSE );

		return AE_TRUE;
	}

	return AE_FALSE;
}
//////////////////////////////////////////////////////////////////////////
ae_bool_t ae_get_movie_composition_node_in_out_time( const aeMovieComposition * _composition, const ae_char_t * _layerName, aeMovieLayerTypeEnum _type, float * _in, float * _out )
{
	const aeMovieInstance * instance = _composition->movie_data->instance;
    	
	const aeMovieNode *it_node = _composition->nodes;
    const aeMovieNode *it_node_end = _composition->nodes + _composition->node_count;
	for( ; it_node != it_node_end; ++it_node )
	{
		const aeMovieNode * node = it_node;

		const aeMovieLayerData * layer = node->layer;

		if( STRNCMP( instance, layer->name, _layerName, AE_MOVIE_MAX_LAYER_NAME ) != 0 )
		{
			continue;
		}

		if( _type != AE_MOVIE_LAYER_TYPE_ANY && layer->type != _type )
		{
			continue;
		}

		*_in = node->in_time;
		*_out = node->out_time;

		return AE_TRUE;
	}

	return AE_FALSE;
}
//////////////////////////////////////////////////////////////////////////