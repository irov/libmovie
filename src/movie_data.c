#	include "movie/movie_data.h"

#	include "memory.h"
#	include "stream.h"

#	include "utils.h"

#	ifndef AE_MOVIE_MAX_COMPOSITION_NAME
#	define AE_MOVIE_MAX_COMPOSITION_NAME 128
#	endif

//////////////////////////////////////////////////////////////////////////
aeMovieData * create_movie_data( const aeMovieInstance * _instance )
{
	aeMovieData * m = NEW( _instance, aeMovieData );

	return m;
}
//////////////////////////////////////////////////////////////////////////
void delete_movie_data( const aeMovieInstance * _instance, const aeMovieData * _movieData )
{
	uint32_t resource_count = _movieData->resource_count;

	for( const aeMovieResource
		**it_resource = _movieData->resources,
		**it_resource_end = _movieData->resources + _movieData->resource_count;
	it_resource != it_resource_end;
	++it_resource )
	{ 
		const aeMovieResource * base_resource = *it_resource;

		uint8_t type = base_resource->type;

		switch( type )
		{
		case AE_MOVIE_RESOURCE_SOCKET_SHAPE:
			{
				const aeMovieResourceSocketShape * resource = (const aeMovieResourceSocketShape *)base_resource;

				DELETEN( _instance, resource->polygons );

				(void)resource;

			}break;
		case AE_MOVIE_RESOURCE_SOLID:
			{
				const aeMovieResourceSolid * resource = (const aeMovieResourceSolid *)base_resource;
				
				(void)resource;

			}break;
		case AE_MOVIE_RESOURCE_VIDEO:
			{
				const aeMovieResourceVideo * resource = (const aeMovieResourceVideo *)base_resource;

				DELETEN( _instance, resource->path );

				(void)resource;

			}break;
		case AE_MOVIE_RESOURCE_SOUND:
			{
				const aeMovieResourceSound * resource = (const aeMovieResourceSound *)base_resource;

				DELETEN( _instance, resource->path );

				(void)resource;

			}break;
		case AE_MOVIE_RESOURCE_IMAGE:
			{
				const aeMovieResourceImage * resource = (const aeMovieResourceImage *)base_resource;

				DELETEN( _instance, resource->path );

				(void)resource;

			}break;
		case AE_MOVIE_RESOURCE_SEQUENCE:
			{
				const aeMovieResourceSequence * resource = (const aeMovieResourceSequence *)base_resource;

				DELETEN( _instance, resource->images );

				(void)resource;

			}break;
		case AE_MOVIE_RESOURCE_ASTRALAX:
			{
				const aeMovieResourceAstralax * resource = (const aeMovieResourceAstralax *)base_resource;

				DELETEN( _instance, resource->atlases );

				(void)resource;
			}break;
		}
	}

	for( const aeMovieCompositionData
		*it_composition = _movieData->compositions,
		*it_composition_end = _movieData->compositions + _movieData->composition_count;
	it_composition != it_composition_end;
	++it_composition )
	{
		const aeMovieCompositionData * composition = it_composition;

		DELETEN( _instance, composition->name );

		if( composition->camera != AE_NULL )
		{
			(void)composition->camera;

			DELETEN( _instance, composition->camera );
		}

		for( const aeMovieLayerData
			*it_layer = composition->layers,
			*it_layer_end = composition->layers + composition->layer_count;
		it_layer != it_layer_end;
		++it_layer )
		{
			const aeMovieLayerData * layer = it_layer;
			
			DELETEN( _instance, layer->name );

			if( layer->timeremap != AE_NULL )
			{
				DELETEN( _instance, layer->timeremap->times );

				DELETEN( _instance, layer->timeremap );
			}

			if( layer->mesh != AE_NULL )
			{
				DELETEN( _instance, layer->mesh->meshes );

				DELETEN( _instance, layer->mesh );
			}

			if( layer->polygon != AE_NULL )
			{
				DELETEN( _instance, layer->polygon->polygons );

				DELETEN( _instance, layer->polygon );
			}

			if( layer->viewport_matte != AE_NULL )
			{
				DELETEN( _instance, layer->viewport_matte->viewports );

				DELETEN( _instance, layer->viewport_matte );
			}

			DELETEN( _instance, layer->property_anchor_point_x );
			DELETEN( _instance, layer->property_anchor_point_y );
			DELETEN( _instance, layer->property_anchor_point_z );
			DELETEN( _instance, layer->property_position_x );
			DELETEN( _instance, layer->property_position_y );
			DELETEN( _instance, layer->property_position_z );
			DELETEN( _instance, layer->property_rotation_x );
			DELETEN( _instance, layer->property_rotation_y );
			DELETEN( _instance, layer->property_rotation_z );
			DELETEN( _instance, layer->property_scale_x );
			DELETEN( _instance, layer->property_scale_y );
			DELETEN( _instance, layer->property_scale_z );
			DELETEN( _instance, layer->property_opacity );
		}

		DELETEN( _instance, composition->layers );
	}

	DELETE( _instance, _movieData );
}
//////////////////////////////////////////////////////////////////////////
static aeMovieResult __load_movie_data_layer_property_zp( const aeMovieStream * _stream, float * _values )
{
	uint32_t count = READZ( _stream );

	float * stream_values = _values;

	uint32_t j = 0;

	for( uint32_t i = 0; i != count; ++i )
	{
		uint8_t block_type;
		READ( _stream, block_type );

		uint8_t block_count = READZ( _stream );

		switch( block_type )
		{
		case 0:
			{
				float block_value;
				READ( _stream, block_value );

				for( uint32_t block_index = 0; block_index != block_count; ++block_index )
				{
					*stream_values++ = block_value;
					++j;
				}
			}break;
		case 1:
			{
				float block_base;
				READ( _stream, block_base );

				float block_add;
				READ( _stream, block_add );

				for( uint32_t block_index = 0; block_index != block_count; ++block_index )
				{
					float block_value = block_base + block_add * block_index;
					*stream_values++ = block_value;
					++j;
				}
			}break;
		case 3:
			{
				for( uint32_t block_index = 0; block_index != block_count; ++block_index )
				{
					float block_value;
					READ( _stream, block_value );

					*stream_values++ = block_value;
					++j;
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
static aeMovieResult __load_movie_data_layer_property( const aeMovieInstance * _instance, const aeMovieStream * _stream, aeMovieLayerData * _layer, uint32_t _count )
{
	uint32_t immutable_property_mask;
	READ( _stream, immutable_property_mask );

	_layer->immutable_property_mask = immutable_property_mask;

#	define AE_MOVIE_STREAM_PROPERTY(Mask, ImmutableName, Name)\
	if( immutable_property_mask & Mask )\
			{\
		READ( _stream, _layer->ImmutableName );\
		_layer->Name = AE_NULL;\
			}\
				else\
			{\
		_layer->ImmutableName = 0.f;\
		_layer->Name = NEWN( _instance, float, _count );\
		if( __load_movie_data_layer_property_zp( _stream, _layer->Name ) == AE_MOVIE_FAILED ) return AE_MOVIE_FAILED;\
			}

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_X, immuttable_anchor_point_x, property_anchor_point_x );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Y, immuttable_anchor_point_y, property_anchor_point_y );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Z, immuttable_anchor_point_z, property_anchor_point_z );

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_X, immuttable_position_x, property_position_x );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_Y, immuttable_position_y, property_position_y );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_Z, immuttable_position_z, property_position_z );

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ROTATION_X, immuttable_rotation_x, property_rotation_x );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ROTATION_Y, immuttable_rotation_y, property_rotation_y );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ROTATION_Z, immuttable_rotation_z, property_rotation_z );

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_X, immuttable_scale_x, property_scale_x );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_Y, immuttable_scale_y, property_scale_y );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_Z, immuttable_scale_z, property_scale_z );

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_OPACITY, immuttable_opacity, property_opacity );

#	undef AE_MOVIE_STREAM_PROPERTY

	return AE_MOVIE_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static aeMovieResult __load_movie_data_composition_camera( const aeMovieInstance * _instance, const aeMovieStream * _stream, aeMovieCompositionData * _composition )
{
	if( READB( _stream ) == AE_TRUE )
	{
		aeMovieCameraData * camera = NEW( _instance, aeMovieCameraData );

		READN( _stream, camera->camera_position, 3 );
		READ( _stream, camera->camera_fov );
		READ( _stream, camera->camera_aspect );
		READ( _stream, camera->camera_width );
		READ( _stream, camera->camera_height );

		_composition->camera = camera;
	}
	else
	{
		_composition->camera = AE_NULL;
	}

	return AE_MOVIE_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static aeMovieResult __load_movie_data_layer( const aeMovieInstance * _instance, const aeMovieStream * _stream, const aeMovieData * _movie, const aeMovieCompositionData * _composition, aeMovieLayerData * _layer )
{
	READ_STRING( _instance, _stream, _layer->name );

	_layer->index = READZ( _stream );
	
	READ( _stream, _layer->type );

	_layer->frame_count = READZ( _stream );

	_layer->timeremap = AE_NULL;
	_layer->mesh = AE_NULL;
	_layer->polygon = AE_NULL;
	_layer->viewport_matte = AE_NULL;

	for( ;; )
	{
		uint8_t extension;
		READ( _stream, extension );

		switch( extension )
		{
		case 0:
			{
			}break;
		case 1:
			{
				_layer->timeremap = NEW( _instance, aeMovieLayerTimeremap );

				_layer->timeremap->immutable = READB( _stream );

				if( _layer->timeremap->immutable == AE_TRUE )
				{
					READ( _stream, _layer->timeremap->immutable_time );
				}
				else
				{
					_layer->timeremap->times = NEWN( _instance, float, _layer->frame_count );

					READN( _stream, _layer->timeremap->times, _layer->frame_count );
				}
			}break;
		case 2:
			{
				_layer->mesh = NEW( _instance, aeMovieLayerMesh );

				_layer->mesh->immutable = READB( _stream );

				if( _layer->mesh->immutable == AE_TRUE )
				{
					READ_MESH( _instance, _stream, &_layer->mesh->immutable_mesh );

					_layer->mesh->meshes = AE_NULL;
				}
				else
				{
					_layer->mesh->meshes = NEWN( _instance, aeMovieMesh, _layer->frame_count );

					for( aeMovieMesh
						*it_mesh = _layer->mesh->meshes,
						*it_mesh_end = _layer->mesh->meshes + _layer->frame_count;
					it_mesh != it_mesh_end;
					++it_mesh )
					{
						READ_MESH( _instance, _stream, it_mesh );
					}
				}
			}
		case 3:
			{
				_layer->polygon = NEW( _instance, aeMovieLayerPolygon );

				_layer->polygon->immutable = READB( _stream );

				if( _layer->polygon->immutable == AE_TRUE )
				{
					READ_POLYGON( _instance, _stream, &_layer->polygon->immutable_polygon );

					_layer->polygon->polygons = AE_NULL;
				}
				else
				{
					_layer->polygon->polygons = NEWN( _instance, aeMoviePolygon, _layer->frame_count );

					for( aeMoviePolygon
						*it_polygon = _layer->polygon->polygons,
						*it_polygon_end = _layer->polygon->polygons + _layer->frame_count;
					it_polygon != it_polygon_end;
					++it_polygon )
					{
						READ_POLYGON( _instance, _stream, it_polygon );
					}
				}
			}
		case 4:
			{
				_layer->viewport_matte = NEW( _instance, aeMovieLayerViewportMatte );

				_layer->viewport_matte->immutable = READB( _stream );

				if( _layer->viewport_matte->immutable == AE_TRUE )
				{
					READ_VIEWPORT( _stream, &_layer->viewport_matte->immutable_viewport );

					_layer->viewport_matte->viewports = AE_NULL;
				}
				else
				{
					_layer->viewport_matte->viewports = NEWN( _instance, aeMovieViewport, _layer->frame_count );

					for( aeMovieViewport
						*it_viewport = _layer->viewport_matte->viewports,
						*it_viewport_end = _layer->viewport_matte->viewports + _layer->frame_count;
					it_viewport != it_viewport_end;
					++it_viewport )
					{
						READ_VIEWPORT( _stream, it_viewport );
					}
				}
			}
		}

		if( extension == 0 )
		{
			break;
		}
	}

	uint8_t is_resource_or_composition;
	READ( _stream, is_resource_or_composition );

	if( is_resource_or_composition == AE_TRUE )
	{
		uint32_t resource_index = READZ( _stream );
		_layer->resource = _movie->resources[resource_index];

		_layer->sub_composition = AE_NULL;
	}
	else
	{
		uint32_t composition_index = READZ( _stream );
		_layer->sub_composition = _movie->compositions + composition_index;

		_layer->resource = AE_NULL;
	}

	uint32_t parent_index = READZ( _stream );

	_layer->parent_index = parent_index;

	READ( _stream, _layer->start_time );
	READ( _stream, _layer->in_time );
	READ( _stream, _layer->out_time );

	READ( _stream, _layer->blend_mode );
	READ( _stream, _layer->params );

	_layer->play_count = READZ( _stream );

	READ( _stream, _layer->stretch );

	if( __load_movie_data_layer_property( _instance, _stream, _layer, _layer->frame_count ) == AE_MOVIE_FAILED )
	{
		return AE_MOVIE_FAILED;
	}

	uint8_t layer_type = _layer->type;
	
	switch( layer_type )
	{
	case AE_MOVIE_LAYER_TYPE_MOVIE:
		{
			_layer->renderable = AE_FALSE;
		}break;
	case AE_MOVIE_LAYER_TYPE_EVENT:
		{
			_layer->renderable = AE_FALSE;
		}break;
	case AE_MOVIE_LAYER_TYPE_SOCKET_SHAPE:
		{
			_layer->renderable = AE_FALSE;
		}break;
	case AE_MOVIE_LAYER_TYPE_SLOT:
		{
			_layer->renderable = AE_TRUE;
		}break;
	case AE_MOVIE_LAYER_TYPE_NULL:
		{
			_layer->renderable = AE_FALSE;
		}break;
	case AE_MOVIE_LAYER_TYPE_SOLID:
		{
			_layer->renderable = AE_TRUE;
		}break;
	case AE_MOVIE_LAYER_TYPE_SEQUENCE:
		{
			_layer->renderable = AE_TRUE;
		}break;
	case AE_MOVIE_LAYER_TYPE_VIDEO:
		{
			_layer->renderable = AE_TRUE;
		}break;
	case AE_MOVIE_LAYER_TYPE_SOUND:
		{
			_layer->renderable = AE_FALSE;
		}break;
	case AE_MOVIE_LAYER_TYPE_ASTRALAX:
		{
			_layer->renderable = AE_TRUE;
		}break;
	case AE_MOVIE_LAYER_TYPE_IMAGE:
		{
			_layer->renderable = AE_TRUE;
		}break;
	case AE_MOVIE_LAYER_TYPE_SUB_MOVIE:
		{
			_layer->renderable = AE_FALSE;
		}break;
	}

	return AE_MOVIE_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static aeMovieResult __load_movie_data_composition( const aeMovieInstance * _instance, const aeMovieStream * _stream, const aeMovieData * _movie, aeMovieCompositionData * _composition )
{
	READ_STRING( _instance, _stream, _composition->name );
		
	READ( _stream, _composition->width );
	READ( _stream, _composition->height );

	READ( _stream, _composition->frameDuration );
	READ( _stream, _composition->duration );

	_composition->flags = 0;

	for( ;; )
	{
		uint8_t flag;
		READ( _stream, flag );

		switch( flag )
		{
		case 0:
			{
			}break;
		case 1:
			{
				READN( _stream, _composition->loopSegment, 2 );

				_composition->flags |= AE_MOVIE_COMPOSITION_LOOP_SEGMENT;
			}break;
		case 2:
			{
				READN( _stream, _composition->anchorPoint, 3 );

				_composition->flags |= AE_MOVIE_COMPOSITION_ANCHOR_POINT;
			}break;
		case 3:
			{
				READN( _stream, _composition->offsetPoint, 3 );

				_composition->flags |= AE_MOVIE_COMPOSITION_OFFSET_POINT;
			}break;
		case 4:
			{
				READN( _stream, _composition->bounds, 4 );

				_composition->flags |= AE_MOVIE_COMPOSITION_BOUNDS;
			}break;
		default:
			{
				return AE_MOVIE_FAILED;
			}break;
		};

		if( flag == 0 )
		{
			break;
		}
	}

	uint32_t layer_count = READZ( _stream );
	
	_composition->layer_count = layer_count;
	_composition->layers = NEWN( _instance, aeMovieLayerData, layer_count );

	for( aeMovieLayerData
		*it_layer = _composition->layers,
		*it_layer_end = _composition->layers + layer_count;
	it_layer != it_layer_end;
	++it_layer )
	{
		aeMovieLayerData * layer = it_layer;

		layer->composition = _composition;

		if( __load_movie_data_layer( _instance, _stream, _movie, _composition, layer ) == AE_MOVIE_FAILED )
		{
			return AE_MOVIE_FAILED;
		}
	}

	if( __load_movie_data_composition_camera( _instance, _stream, _composition ) == AE_MOVIE_FAILED )
	{
		return AE_MOVIE_FAILED;
	}

	return AE_MOVIE_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
aeMovieResult load_movie_data( const aeMovieInstance * _instance, const aeMovieStream * _stream, aeMovieData * _movie, movie_data_resource_provider_t _provider, void * _data )
{
	char magic[4];
	READN( _stream, magic, 4 );

	if( magic[0] != 'A' ||
		magic[1] != 'E' ||
		magic[2] != 'M' ||
		magic[3] != '1' )
	{
		return AE_MOVIE_FAILED;
	}

	uint32_t version;
	READ( _stream, version );

	if( version != ae_movie_version )
	{
		return AE_MOVIE_FAILED;
	}

	READ_STRING( _instance, _stream, _movie->name );

	uint32_t resource_count = READZ( _stream );

	_movie->resource_count = resource_count;
	_movie->resources = NEWN( _instance, aeMovieResource *, resource_count );

	uint32_t aa = 0;

	for( aeMovieResource
		**it_resource = _movie->resources,
		**it_resource_end = _movie->resources + resource_count;
	it_resource != it_resource_end;
	++it_resource )
	{
		uint8_t type;
		READ( _stream, type );

		switch( type )
		{
		case AE_MOVIE_RESOURCE_SOCKET_SHAPE:
			{
				aeMovieResourceSocketShape * resource = NEW( _instance, aeMovieResourceSocketShape );

				uint32_t polygon_count = READZ( _stream );

				resource->polygons = NEWN( _instance, aeMoviePolygon, polygon_count );

				for( aeMoviePolygon
					*it_polygon = resource->polygons,
					*it_polygon_end = resource->polygons + polygon_count;
				it_polygon != it_polygon_end;
				++it_polygon )
				{
					READ_POLYGON( _instance, _stream, it_polygon );
				}

				resource->base.data = AE_NULL;

				*it_resource = (aeMovieResource *)resource;
			}break;
		case AE_MOVIE_RESOURCE_SOLID:
			{
				aeMovieResourceSolid * resource = NEW( _instance, aeMovieResourceSolid );

				READ( _stream, resource->width );
				READ( _stream, resource->height );
				READ( _stream, resource->r );
				READ( _stream, resource->g );
				READ( _stream, resource->b );

				resource->base.data = AE_NULL;

				*it_resource = (aeMovieResource *)resource;
			}break;
		case AE_MOVIE_RESOURCE_VIDEO:
			{
				aeMovieResourceVideo * resource = NEW( _instance, aeMovieResourceVideo );

				READ_STRING( _instance, _stream, resource->path );

				READ( _stream, resource->alpha );
				READ( _stream, resource->frameRate );
				READ( _stream, resource->duration );

				resource->base.data = (*_provider)(type, resource->path, _data);

				*it_resource = (aeMovieResource *)resource;
			}break;
		case AE_MOVIE_RESOURCE_SOUND:
			{
				aeMovieResourceSound * resource = NEW( _instance, aeMovieResourceSound );

				READ_STRING( _instance, _stream, resource->path );

				READ( _stream, resource->duration );

				resource->base.data = (*_provider)(type, resource->path, _data);

				*it_resource = (aeMovieResource *)resource;
			}break;
		case AE_MOVIE_RESOURCE_IMAGE:
			{
				aeMovieResourceImage * resource = NEW( _instance, aeMovieResourceImage );

				READ_STRING( _instance, _stream, resource->path );

				READ( _stream, resource->width);
				READ( _stream, resource->height );
				READ( _stream, resource->offset_x );
				READ( _stream, resource->offset_y );

				resource->base.data = (*_provider)(type, resource->path, _data);

				*it_resource = (aeMovieResource *)resource;
			}break;
		case AE_MOVIE_RESOURCE_SEQUENCE:
			{
				aeMovieResourceSequence * resource = NEW( _instance, aeMovieResourceSequence );

				READ( _stream, resource->frameDuration );

				uint32_t sequence_count = READZ( _stream );

				resource->images = NEWN( _instance, aeMovieResourceImage *, sequence_count );

				for( aeMovieResourceImage
					**it_image = resource->images,
					**it_image_end = resource->images + sequence_count;
				it_image != it_image_end;
				++it_image )
				{
					uint32_t resource_id = READZ( _stream );

					*it_image = (aeMovieResourceImage *)_movie->resources[resource_id];
				}

				resource->base.data = AE_NULL;

				*it_resource = (aeMovieResource *)resource;
			}break;
		case AE_MOVIE_RESOURCE_ASTRALAX:
			{
				aeMovieResourceAstralax * resource = NEW( _instance, aeMovieResourceAstralax );

				READ_STRING( _instance, _stream, resource->path );

				uint32_t atlas_count = READZ( _stream );

				resource->atlas_count = atlas_count;

				resource->atlases = NEWN( _instance, aeMovieResourceImage *, atlas_count );

				for( aeMovieResourceImage
					**it_image = resource->atlases,
					**it_image_end = resource->atlases + atlas_count;
				it_image != it_image_end;
				++it_image )
				{
					uint32_t resource_id = READZ( _stream );

					*it_image = (aeMovieResourceImage *)_movie->resources[resource_id];
				}

				resource->base.data = (*_provider)(type, resource->path, _data);

				*it_resource = (aeMovieResource *)resource;
			}break;
		default:
			{
				return AE_MOVIE_FAILED;
			}break;
		}

		aeMovieResource * new_resource = (*it_resource);

		new_resource->type = type;
	}

	uint32_t composition_count = READZ( _stream );

	_movie->composition_count = composition_count;
	_movie->compositions = NEWN( _instance, aeMovieCompositionData, composition_count );

	for( aeMovieCompositionData
		*it_composition = _movie->compositions,
		*it_composition_end = _movie->compositions + composition_count;
	it_composition != it_composition_end;
	++it_composition )
	{
		if( __load_movie_data_composition( _instance, _stream, _movie, it_composition ) == AE_MOVIE_FAILED )
		{
			return AE_MOVIE_FAILED;
		}
	}

	return AE_MOVIE_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
const aeMovieCompositionData * get_movie_composition_data( const aeMovieData * _movie, const char * _name )
{
	for( const aeMovieCompositionData
		*it_composition = _movie->compositions,
		*it_composition_end = _movie->compositions + _movie->composition_count;
	it_composition != it_composition_end;
	++it_composition )
	{
		const aeMovieCompositionData * composition = it_composition;

		if( ae_strncmp( composition->name, _name, AE_MOVIE_MAX_COMPOSITION_NAME ) != 0 )
		{
			continue;
		}

		return composition;
	}

	return AE_NULL;
}
//////////////////////////////////////////////////////////////////////////
uint32_t get_movie_composition_data_node_count( const aeMovieData * _movie, const aeMovieCompositionData * _compositionData )
{
	uint32_t count = _compositionData->layer_count;

	for( const aeMovieLayerData
		*it_layer = _compositionData->layers,
		*it_layer_end = _compositionData->layers + _compositionData->layer_count;
	it_layer != it_layer_end;
	++it_layer )
	{
		const aeMovieLayerData * layer = it_layer;

		uint8_t layer_type = layer->type;

		switch( layer_type )
		{			
		case AE_MOVIE_LAYER_TYPE_MOVIE:
			{
				uint32_t movie_layer_count = get_movie_composition_data_node_count( _movie, layer->sub_composition );

				count += movie_layer_count;
			}break;
		case AE_MOVIE_LAYER_TYPE_SUB_MOVIE:
			{
				uint32_t movie_layer_count = get_movie_composition_data_node_count( _movie, layer->sub_composition );

				count += movie_layer_count;
			}break;
		}
	}

	return count;
}
//////////////////////////////////////////////////////////////////////////