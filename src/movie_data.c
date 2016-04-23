#	include "movie/movie_data.h"

#	include "movie_memory.h"
#	include "movie_stream.h"

#	include "movie_utils.h"

#	ifndef AE_MOVIE_MAX_COMPOSITION_NAME
#	define AE_MOVIE_MAX_COMPOSITION_NAME 128
#	endif

//////////////////////////////////////////////////////////////////////////
aeMovieData * create_movie_data( const aeMovieInstance * _instance )
{
	aeMovieData * m = NEW( _instance, aeMovieData );

	m->instance = _instance;

	return m;
}
//////////////////////////////////////////////////////////////////////////
static void delete_movie_layer_transformation( const aeMovieInstance * _instance, const aeMovieLayerTransformation * _transformation )
{
	DELETEN( _instance, _transformation->property_anchor_point_x );
	DELETEN( _instance, _transformation->property_anchor_point_y );
	DELETEN( _instance, _transformation->property_anchor_point_z );
	DELETEN( _instance, _transformation->property_position_x );
	DELETEN( _instance, _transformation->property_position_y );
	DELETEN( _instance, _transformation->property_position_z );
	DELETEN( _instance, _transformation->property_rotation_x );
	DELETEN( _instance, _transformation->property_rotation_y );
	DELETEN( _instance, _transformation->property_rotation_z );
	DELETEN( _instance, _transformation->property_scale_x );
	DELETEN( _instance, _transformation->property_scale_y );
	DELETEN( _instance, _transformation->property_scale_z );
	DELETEN( _instance, _transformation->property_opacity );
}
//////////////////////////////////////////////////////////////////////////
void delete_movie_data( const aeMovieData * _movieData )
{
	const aeMovieInstance * instance = _movieData->instance;

	uint32_t resource_count = _movieData->resource_count;

	for( const aeMovieResource
		*const * it_resource = _movieData->resources,
		*const * it_resource_end = _movieData->resources + _movieData->resource_count;
	it_resource != it_resource_end;
	++it_resource )
	{ 
		const aeMovieResource * base_resource = *it_resource;

		uint8_t type = base_resource->type;

		switch( type )
		{
		case AE_MOVIE_RESOURCE_SOCKET:
			{
				const aeMovieResourceSocket * resource = (const aeMovieResourceSocket *)base_resource;
				
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

				DELETEN( instance, resource->path );

				(void)resource;

			}break;
		case AE_MOVIE_RESOURCE_SOUND:
			{
				const aeMovieResourceSound * resource = (const aeMovieResourceSound *)base_resource;

				DELETEN( instance, resource->path );

				(void)resource;

			}break;
		case AE_MOVIE_RESOURCE_IMAGE:
			{
				const aeMovieResourceImage * resource = (const aeMovieResourceImage *)base_resource;

				DELETEN( instance, resource->path );

				(void)resource;

			}break;
		case AE_MOVIE_RESOURCE_SEQUENCE:
			{
				const aeMovieResourceSequence * resource = (const aeMovieResourceSequence *)base_resource;

				DELETEN( instance, resource->images );

				(void)resource;

			}break;
		case AE_MOVIE_RESOURCE_PARTICLE:
			{
				const aeMovieResourceParticle * resource = (const aeMovieResourceParticle *)base_resource;

				DELETEN( instance, resource->path );

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

		DELETEN( instance, composition->name );

		for( const aeMovieLayerData
			*it_layer = composition->layers,
			*it_layer_end = composition->layers + composition->layer_count;
		it_layer != it_layer_end;
		++it_layer )
		{
			const aeMovieLayerData * layer = it_layer;
			
			DELETEN( instance, layer->name );

			if( layer->timeremap != AE_NULL )
			{
				DELETEN( instance, layer->timeremap->times );

				DELETEN( instance, layer->timeremap );
			}

			if( layer->mesh != AE_NULL )
			{
				DELETEN( instance, layer->mesh->meshes );

				DELETEN( instance, layer->mesh );
			}

			delete_movie_layer_transformation( instance, &layer->transformation );
		}

		DELETEN( instance, composition->layers );
	}

	DELETE( instance, _movieData );
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
static aeMovieResult __load_movie_data_layer_property( const aeMovieInstance * _instance, const aeMovieStream * _stream, aeMovieLayerTransformation * _transformation, uint32_t _count )
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

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ROTATION_X, immuttable_rotation_x, property_rotation_x );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ROTATION_Y, immuttable_rotation_y, property_rotation_y );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ROTATION_Z, immuttable_rotation_z, property_rotation_z );

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_OPACITY, immuttable_opacity, property_opacity );

#	undef AE_MOVIE_STREAM_PROPERTY

	return AE_MOVIE_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static void __load_movie_data_composition_camera( const aeMovieStream * _stream, aeMovieCompositionData * _compostionData )
{
	READ( _stream, _compostionData->cameraZoom );
}
//////////////////////////////////////////////////////////////////////////
static aeMovieResult __load_movie_data_layer( const aeMovieData * _movieData, const aeMovieStream * _stream, const aeMovieCompositionData * _composition, aeMovieLayerData * _layer )
{
	READ_STRING( _movieData->instance, _stream, _layer->name );

	_layer->index = READZ( _stream );
	
	READ( _stream, _layer->type );

	_layer->is_track_matte = READB( _stream );
	_layer->has_track_matte = READB( _stream );

	_layer->frame_count = READZ( _stream );

	_layer->timeremap = AE_NULL;
	_layer->mesh = AE_NULL;
	
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
				_layer->timeremap = NEW( _movieData->instance, aeMovieLayerTimeremap );

				_layer->timeremap->immutable = READB( _stream );

				if( _layer->timeremap->immutable == AE_TRUE )
				{
					READ( _stream, _layer->timeremap->immutable_time );
				}
				else
				{
					_layer->timeremap->times = NEWN( _movieData->instance, float, _layer->frame_count );

					READN( _stream, _layer->timeremap->times, _layer->frame_count );
				}
			}break;
		case 2:
			{
				_layer->mesh = NEW( _movieData->instance, aeMovieLayerMesh );

				_layer->mesh->immutable = READB( _stream );

				if( _layer->mesh->immutable == AE_TRUE )
				{
					READ_MESH( _movieData->instance, _stream, &_layer->mesh->immutable_mesh );

					_layer->mesh->meshes = AE_NULL;
				}
				else
				{
					_layer->mesh->meshes = NEWN( _movieData->instance, aeMovieMesh, _layer->frame_count );

					for( aeMovieMesh
						*it_mesh = _layer->mesh->meshes,
						*it_mesh_end = _layer->mesh->meshes + _layer->frame_count;
					it_mesh != it_mesh_end;
					++it_mesh )
					{
						READ_MESH( _movieData->instance, _stream, it_mesh );
					}
				}
			}break;
		//case 4:
		//	{
		//		_layer->track_matte = NEW( _movieData->instance, aeMovieLayerTrackMatte );

		//		_layer->track_matte->track_image = READB( _stream );

		//		if( _layer->track_matte->track_image == AE_TRUE )
		//		{
		//			READ_STRING( _movieData->instance, _stream, _layer->track_matte->image_path );

		//			_layer->track_matte->solid_width = 0.f;
		//			_layer->track_matte->solid_height = 0.f;
		//		}
		//		else
		//		{
		//			_layer->track_matte->image_path = AE_NULL;

		//			READ( _stream, _layer->track_matte->solid_width );
		//			READ( _stream, _layer->track_matte->solid_height );					
		//		}

		//		_layer->track_matte->track_count = READZ( _stream );

		//		__load_movie_data_layer_property( _movieData->instance, _stream, &_layer->track_matte->track_transformation, _layer->track_matte->track_count );
		//	}break;
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

		if( resource_index == 0 )
		{
			_layer->resource = AE_NULL;
		}
		else
		{
			_layer->resource = _movieData->resources[resource_index - 1];
		}

		_layer->sub_composition = AE_NULL;
	}
	else
	{
		uint32_t composition_index = READZ( _stream );
		_layer->sub_composition = _movieData->compositions + composition_index;

		_layer->resource = AE_NULL;
	}

	uint32_t parent_index = READZ( _stream );

	_layer->parent_index = parent_index;

	_layer->reverse_time = READB( _stream );

	READ( _stream, _layer->start_time );
	READ( _stream, _layer->in_time );
	READ( _stream, _layer->out_time );

	if( _layer->in_time < 0.f )
	{
		_layer->start_time -= _layer->in_time;
		_layer->in_time = 0.f;
	}

	READ( _stream, _layer->blend_mode );	
	_layer->threeD = READB( _stream );
	READ( _stream, _layer->params );

	_layer->play_count = READZ( _stream );

	READ( _stream, _layer->stretch );
	
	if( __load_movie_data_layer_property( _movieData->instance, _stream, &_layer->transformation, _layer->frame_count ) == AE_MOVIE_FAILED )
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
	case AE_MOVIE_LAYER_TYPE_SOCKET:
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
	case AE_MOVIE_LAYER_TYPE_PARTICLE:
		{
			_layer->renderable = AE_TRUE;
		}break;
	case AE_MOVIE_LAYER_TYPE_IMAGE:
		{
			_layer->renderable = AE_TRUE;
		}break;
	case AE_MOVIE_LAYER_TYPE_SUB_MOVIE:
		{
			_layer->renderable = AE_TRUE;
		}break;
	}

	return AE_MOVIE_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static aeMovieResult __load_movie_data_composition( const aeMovieData * _movieData, const aeMovieStream * _stream, aeMovieCompositionData * _compositionData )
{
	READ_STRING( _movieData->instance, _stream, _compositionData->name );
		
	READ( _stream, _compositionData->width );
	READ( _stream, _compositionData->height );

	READ( _stream, _compositionData->frameDuration );
	READ( _stream, _compositionData->duration );

	_compositionData->frameCount = (uint32_t)( _compositionData->duration / _compositionData->frameDuration + 0.5f );

	_compositionData->flags = 0;

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
				READN( _stream, _compositionData->loopSegment, 2 );

				_compositionData->flags |= AE_MOVIE_COMPOSITION_LOOP_SEGMENT;
			}break;
		case 2:
			{
				READN( _stream, _compositionData->anchorPoint, 3 );

				_compositionData->flags |= AE_MOVIE_COMPOSITION_ANCHOR_POINT;
			}break;
		case 3:
			{
				READN( _stream, _compositionData->offsetPoint, 3 );

				_compositionData->flags |= AE_MOVIE_COMPOSITION_OFFSET_POINT;
			}break;
		case 4:
			{
				READN( _stream, _compositionData->bounds, 4 );

				_compositionData->flags |= AE_MOVIE_COMPOSITION_BOUNDS;
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

	_compositionData->has_threeD = READB( _stream );

	if( _compositionData->has_threeD == AE_TRUE )
	{
		__load_movie_data_composition_camera( _stream, _compositionData );
	}	
	
	uint32_t layer_count = READZ( _stream );
	
	_compositionData->layer_count = layer_count;
	_compositionData->layers = NEWN( _movieData->instance, aeMovieLayerData, layer_count );

	for( aeMovieLayerData
		*it_layer = _compositionData->layers,
		*it_layer_end = _compositionData->layers + layer_count;
	it_layer != it_layer_end;
	++it_layer )
	{
		aeMovieLayerData * layer = it_layer;

		layer->composition = _compositionData;

		if( __load_movie_data_layer( _movieData, _stream, _compositionData, layer ) == AE_MOVIE_FAILED )
		{
			return AE_MOVIE_FAILED;
		}
	}

	return AE_MOVIE_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
aeMovieResult load_movie_data( aeMovieData * _movieData, const aeMovieStream * _stream, movie_data_resource_provider_t _provider, void * _data )
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

	READ_STRING( _movieData->instance, _stream, _movieData->name );

	uint32_t resource_count = READZ( _stream );

	_movieData->resource_count = resource_count;
	_movieData->resources = NEWN( _movieData->instance, aeMovieResource *, resource_count );

	for( aeMovieResource
		**it_resource = _movieData->resources,
		**it_resource_end = _movieData->resources + resource_count;
	it_resource != it_resource_end;
	++it_resource )
	{
		uint8_t type;
		READ( _stream, type );

		switch( type )
		{
		case AE_MOVIE_RESOURCE_SOCKET:
			{
				aeMovieResourceSocket * resource = NEW( _movieData->instance, aeMovieResourceSocket );

				uint32_t polygon_count = READZ( _stream );

				READ_POLYGON( _movieData->instance, _stream, &resource->polygon );

				resource->data = AE_NULL;

				*it_resource = (aeMovieResource *)resource;
			}break;
		case AE_MOVIE_RESOURCE_SOLID:
			{
				aeMovieResourceSolid * resource = NEW( _movieData->instance, aeMovieResourceSolid );

				READ( _stream, resource->width );
				READ( _stream, resource->height );
				READ( _stream, resource->r );
				READ( _stream, resource->g );
				READ( _stream, resource->b );

				resource->data = AE_NULL;

				*it_resource = (aeMovieResource *)resource;
			}break;
		case AE_MOVIE_RESOURCE_VIDEO:
			{
				aeMovieResourceVideo * resource = NEW( _movieData->instance, aeMovieResourceVideo );

				READ_STRING( _movieData->instance, _stream, resource->path );

				READ( _stream, resource->width );
				READ( _stream, resource->height );
				READ( _stream, resource->alpha );
				READ( _stream, resource->frameRate );
				READ( _stream, resource->duration );

				*it_resource = (aeMovieResource *)resource;
			}break;
		case AE_MOVIE_RESOURCE_SOUND:
			{
				aeMovieResourceSound * resource = NEW( _movieData->instance, aeMovieResourceSound );

				READ_STRING( _movieData->instance, _stream, resource->path );

				READ( _stream, resource->duration );

				*it_resource = (aeMovieResource *)resource;
			}break;
		case AE_MOVIE_RESOURCE_IMAGE:
			{
				aeMovieResourceImage * resource = NEW( _movieData->instance, aeMovieResourceImage );

				READ_STRING( _movieData->instance, _stream, resource->path );

				READ( _stream, resource->base_width );
				READ( _stream, resource->base_height );
				READ( _stream, resource->trim_width);
				READ( _stream, resource->trim_height );
				READ( _stream, resource->offset_x );
				READ( _stream, resource->offset_y );								

				*it_resource = (aeMovieResource *)resource;
			}break;
		case AE_MOVIE_RESOURCE_SEQUENCE:
			{
				aeMovieResourceSequence * resource = NEW( _movieData->instance, aeMovieResourceSequence );

				READ( _stream, resource->frameDuration );

				uint32_t image_count = READZ( _stream );

				resource->image_count = image_count;
				resource->images = NEWN( _movieData->instance, aeMovieResourceImage *, image_count );

				for( aeMovieResourceImage
					**it_image = resource->images,
					**it_image_end = resource->images + image_count;
				it_image != it_image_end;
				++it_image )
				{
					uint32_t resource_id = READZ( _stream );

					*it_image = (aeMovieResourceImage *)_movieData->resources[resource_id];
				}

				resource->data = AE_NULL;

				*it_resource = (aeMovieResource *)resource;
			}break;
		case AE_MOVIE_RESOURCE_PARTICLE:
			{
				aeMovieResourceParticle * resource = NEW( _movieData->instance, aeMovieResourceParticle );

				READ_STRING( _movieData->instance, _stream, resource->path );

				*it_resource = (aeMovieResource *)resource;
			}break;
		default:
			{
				return AE_MOVIE_FAILED;
			}break;
		}

		aeMovieResource * new_resource = (*it_resource);

		new_resource->type = type;

		new_resource->data = (*_provider)(new_resource, _data);
	}

	uint32_t composition_count = READZ( _stream );

	_movieData->composition_count = composition_count;
	_movieData->compositions = NEWN( _movieData->instance, aeMovieCompositionData, composition_count );

	for( aeMovieCompositionData
		*it_composition = _movieData->compositions,
		*it_composition_end = _movieData->compositions + composition_count;
	it_composition != it_composition_end;
	++it_composition )
	{
		if( __load_movie_data_composition( _movieData, _stream, it_composition ) == AE_MOVIE_FAILED )
		{
			return AE_MOVIE_FAILED;
		}
	}

	return AE_MOVIE_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
const aeMovieCompositionData * get_movie_composition_data( const aeMovieData * _movieData, const char * _name )
{
	for( const aeMovieCompositionData
		*it_composition = _movieData->compositions,
		*it_composition_end = _movieData->compositions + _movieData->composition_count;
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