#	include "movie/movie_data.h"

#	include "memory.h"
#	include "stream.h"

#	include <memory.h>

//////////////////////////////////////////////////////////////////////////
aeMovieData * create_movie_data( const aeMovieInstance * _instance )
{
	aeMovieData * m = NEW( _instance, aeMovieData );

	return m;
}
//////////////////////////////////////////////////////////////////////////
void delete_movie_data( const aeMovieInstance * _instance, aeMovieData * _movieData )
{
	DELETE( _instance, _movieData );
}
//////////////////////////////////////////////////////////////////////////
static aeMovieResult __load_movie_data_layer( const aeMovieInstance * _instance, const aeMovieStream * _stream, const aeMovieData * _movie, const aeMovieCompositionData * _composition, aeMovieLayerData * _layer )
{
	READ_STRING( _instance, _stream, _layer->name );

	_layer->index = READZ( _stream );
	_layer->type = READZ( _stream );

	_layer->immutable = READB( _stream );
	_layer->frame_count = READZ( _stream );

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

				if( _layer->immutable == AE_TRUE )
				{
					READ( _stream, _layer->timeremap->immutable_time );

					_layer->timeremap->times = AE_NULL;
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
	}

	uint32_t resource_index = READZ( _stream );

	_layer->resource = _movie->resources[resource_index];

	uint32_t parent_index = READZ( _stream );

	_layer->parent = _composition->layers + parent_index;

	uint8_t three_d = READB( _stream );

	READ_PROPERTY( _instance, _stream, &_layer->properties, _layer->frame_count );

	return AE_MOVIE_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
aeMovieResult load_movie_data( const aeMovieInstance * _instance, const aeMovieStream * _stream, aeMovieData * _movie )
{
	char magic[4];
	READN( _stream, magic, 4 );

	if( memcmp( magic, "AEM1", 4 ) != 0 )
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

	_movie->resources = NEWN( _instance, aeMovieResource *, resource_count );

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
		case 1:
			{
				aeMovieResourceInternal * resource = NEW( _instance, aeMovieResourceInternal );

				READ_STRING( _instance, _stream, resource->name );

				*it_resource = (aeMovieResource *)resource;
			}break;
		case 2:
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

				*it_resource = (aeMovieResource *)resource;
			}break;
		case 3:
			{
				aeMovieResourceSocketImage * resource = NEW( _instance, aeMovieResourceSocketImage );

				READ_STRING( _instance, _stream, resource->path );

				*it_resource = (aeMovieResource *)resource;
			}break;
		case 4:
			{
				aeMovieResourceSolid * resource = NEW( _instance, aeMovieResourceSolid );

				READ( _stream, resource->width );
				READ( _stream, resource->height );
				READ( _stream, resource->r );
				READ( _stream, resource->g );
				READ( _stream, resource->b );

				*it_resource = (aeMovieResource *)resource;
			}break;
		case 5:
			{
				aeMovieResourceVideo * resource = NEW( _instance, aeMovieResourceVideo );

				READ_STRING( _instance, _stream, resource->path );

				READ( _stream, resource->alpha );
				READ( _stream, resource->frameRate );
				READ( _stream, resource->duration );

				*it_resource = (aeMovieResource *)resource;
			}break;
		case 6:
			{
				aeMovieResourceSound * resource = NEW( _instance, aeMovieResourceSound );

				READ_STRING( _instance, _stream, resource->path );

				READ( _stream, resource->duration );

				*it_resource = (aeMovieResource *)resource;
			}break;
		case 7:
			{
				aeMovieResourceImage * resource = NEW( _instance, aeMovieResourceImage );

				READ_STRING( _instance, _stream, resource->path );

				READ( _stream, resource->base_width );
				READ( _stream, resource->base_height );
				READ( _stream, resource->trim_width);
				READ( _stream, resource->trim_height );
				READ( _stream, resource->trim_offset_x );
				READ( _stream, resource->trim_offset_y );

				*it_resource = (aeMovieResource *)resource;
			}break;
		case 8:
			{
				aeMovieResourceImageSequence * resource = NEW( _instance, aeMovieResourceImageSequence );

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

				*it_resource = (aeMovieResource *)resource;
			}break;
		}

		(*it_resource)->type = type;
	}

	uint32_t composition_count = READZ( _stream );

	_movie->compositions = NEWN( _instance, aeMovieCompositionData, composition_count );

	for( aeMovieCompositionData
		*it_composition = _movie->compositions,
		*it_composition_end = _movie->compositions + composition_count;
	it_composition != it_composition_end;
	++it_composition )
	{
		READ_STRING( _instance, _stream, it_composition->name );

		READ( _stream, it_composition->width );
		READ( _stream, it_composition->height );

		READ( _stream, it_composition->frameDuration );
		READ( _stream, it_composition->duration );

		it_composition->flags = 0;

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
					READN( _stream, it_composition->loopSegment, 2 );

					it_composition->flags |= AE_MOVIE_COMPOSITION_LOOP_SEGMENT;
				}break;
			case 2:
				{
					READN( _stream, it_composition->anchorPoint, 3 );

					it_composition->flags |= AE_MOVIE_COMPOSITION_ANCHOR_POINT;
				}break;
			case 3:
				{
					READN( _stream, it_composition->offsetPoint, 3 );

					it_composition->flags |= AE_MOVIE_COMPOSITION_OFFSET_POINT;
				}break;
			case 4:
				{
					READN( _stream, it_composition->bounds, 4 );

					it_composition->flags |= AE_MOVIE_COMPOSITION_BOUNDS;
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

		it_composition->layers = NEWN( _instance, aeMovieLayerData, layer_count );

		for( uint32_t layer_index = 0; layer_index != layer_count; ++layer_index )
		{
			aeMovieLayerData * layer = it_composition->layers + layer_index;

			if( __load_movie_data_layer( _instance, _stream, _movie, it_composition, layer ) == AE_MOVIE_FAILED )
			{
				return AE_MOVIE_FAILED;
			}
		}
	}

	return AE_MOVIE_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////