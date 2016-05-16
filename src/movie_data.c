#	include "movie/movie_data.h"

#	include "movie_transformation.h"
#	include "movie_memory.h"
#	include "movie_stream.h"

#	include "movie_utils.h"

#	ifndef AE_MOVIE_MAX_COMPOSITION_NAME
#	define AE_MOVIE_MAX_COMPOSITION_NAME 128
#	endif

//////////////////////////////////////////////////////////////////////////
aeMovieData * ae_create_movie_data( const aeMovieInstance * _instance )
{
	aeMovieData * m = NEW( _instance, aeMovieData );

	m->instance = _instance;

	return m;
}
//////////////////////////////////////////////////////////////////////////
void ae_delete_movie_data( const aeMovieData * _movieData )
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
		case AE_MOVIE_RESOURCE_SHAPE:
			{
				const aeMovieResourceShape * resource = (const aeMovieResourceShape *)base_resource;

				DELETEN( instance, resource->meshes );

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

			delete_movie_layer_transformation( instance, layer->transformation );

			DELETE( instance, layer->transformation );
		}

		DELETEN( instance, composition->layers );
	}

	DELETE( instance, _movieData );
}
//////////////////////////////////////////////////////////////////////////
static void __load_movie_data_composition_camera( const aeMovieStream * _stream, aeMovieCompositionData * _compostionData )
{
	READ( _stream, _compostionData->cameraZoom );
}
//////////////////////////////////////////////////////////////////////////
static aeMovieResult __load_movie_data_layer( const aeMovieData * _movieData, const aeMovieStream * _stream, const aeMovieCompositionData * _compositionData, aeMovieLayerData * _layer )
{
	READ_STRING( _movieData->instance, _stream, _layer->name );

	_layer->index = READZ( _stream );
	
	_layer->is_track_matte = READB( _stream );
	_layer->has_track_matte = READB( _stream );

	READ( _stream, _layer->type );

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
		default:
			{
				return AE_MOVIE_FAILED;
			}break;
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

	if( _layer->out_time > _compositionData->duration )
	{
		_layer->out_time = _compositionData->duration;
	}

	READ( _stream, _layer->blend_mode );	
	_layer->threeD = READB( _stream );
	READ( _stream, _layer->params );

	_layer->play_count = READZ( _stream );

	READ( _stream, _layer->stretch );

	_layer->transformation = NEW( _movieData->instance, aeMovieLayerTransformation );
	
	if( load_movie_layer_transformation( _movieData->instance, _stream, _layer->transformation, _layer->frame_count ) == AE_MOVIE_FAILED )
	{
		return AE_MOVIE_FAILED;
	}

	if( _layer->is_track_matte == AE_TRUE )
	{
		_layer->renderable = AE_FALSE;
	}
	else
	{
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
		case AE_MOVIE_LAYER_TYPE_SHAPE:
			{
				_layer->renderable = AE_TRUE;
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
		default:
			{
				return AE_MOVIE_FAILED;
			}break;
		}
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

	_compositionData->loopSegment[0] = 0.f;
	_compositionData->loopSegment[1] = _compositionData->duration;

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
aeMovieResult ae_load_movie_data( aeMovieData * _movieData, const aeMovieStream * _stream, ae_movie_data_resource_provider_t _provider, void * _data )
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

	for( const aeMovieResource
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

				ae_bool_t polygon_immutable = READB( _stream );

				if( polygon_immutable == AE_TRUE )
				{
					READ_POLYGON( _movieData->instance, _stream, &resource->immutable_polygon );
				}
				else
				{
					uint32_t polygon_count = READZ( _stream );

					resource->polygons = NEWN( _movieData->instance, aeMoviePolygon, polygon_count );
					
					for( aeMoviePolygon
						*it_polygon = resource->polygons,
						*it_polygon_end = resource->polygons + polygon_count;
					it_polygon != it_polygon_end;
					++it_polygon )
					{
						READ_POLYGON( _movieData->instance, _stream, it_polygon );
					}
				}

				*it_resource = (aeMovieResource *)resource;

				resource->type = type;
				resource->data = (*_provider)(*it_resource, _data);
			}break;
		case AE_MOVIE_RESOURCE_SHAPE:
			{
				aeMovieResourceShape * resource = NEW( _movieData->instance, aeMovieResourceShape );

				READ( _stream, resource->r );
				READ( _stream, resource->g );
				READ( _stream, resource->b );
				READ( _stream, resource->a );

				resource->immutable = READB( _stream );

				if( resource->immutable == AE_TRUE )
				{
					READ_MESH( _movieData->instance, _stream, &resource->immutable_mesh );
				}
				else
				{
					uint32_t mesh_count = READZ( _stream );

					resource->meshes = NEWN( _movieData->instance, aeMovieMesh, mesh_count );

					for( aeMovieMesh
						*it_mesh = resource->meshes,
						*it_mesh_end = resource->meshes + mesh_count;
					it_mesh != it_mesh_end;
					++it_mesh )
					{
						READ_MESH( _movieData->instance, _stream, it_mesh );
					}
				}

				*it_resource = (aeMovieResource *)resource;

				resource->type = type;
				resource->data = (*_provider)(*it_resource, _data);
			}break;
		case AE_MOVIE_RESOURCE_SOLID:
			{
				aeMovieResourceSolid * resource = NEW( _movieData->instance, aeMovieResourceSolid );

				READ( _stream, resource->width );
				READ( _stream, resource->height );
				READ( _stream, resource->r );
				READ( _stream, resource->g );
				READ( _stream, resource->b );

				*it_resource = (aeMovieResource *)resource;

				resource->type = type;
				resource->data = (*_provider)(*it_resource, _data);
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

				resource->type = type;
				resource->data = (*_provider)(*it_resource, _data);
			}break;
		case AE_MOVIE_RESOURCE_SOUND:
			{
				aeMovieResourceSound * resource = NEW( _movieData->instance, aeMovieResourceSound );

				READ_STRING( _movieData->instance, _stream, resource->path );

				READ( _stream, resource->duration );

				*it_resource = (aeMovieResource *)resource;

				resource->type = type;
				resource->data = (*_provider)(*it_resource, _data);
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

				resource->type = type;
				resource->data = (*_provider)(*it_resource, _data);
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

				resource->type = type;
				resource->data = (*_provider)(*it_resource, _data);
			}break;
		case AE_MOVIE_RESOURCE_PARTICLE:
			{
				aeMovieResourceParticle * resource = NEW( _movieData->instance, aeMovieResourceParticle );

				READ_STRING( _movieData->instance, _stream, resource->path );

				*it_resource = (aeMovieResource *)resource;

				resource->type = type;
				resource->data = (*_provider)(*it_resource, _data);
			}break;
		default:
			{
				return AE_MOVIE_FAILED;
			}break;
		}
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
const aeMovieCompositionData * ae_get_movie_composition_data( const aeMovieData * _movieData, const char * _name )
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