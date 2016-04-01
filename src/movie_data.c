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
aeMovieResult load_movie_data( const aeMovieInstance * _instance, aeMovieData * _movie, const aeMovieStream * _stream )
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

	READSTR( _instance, _stream, _movie->name );

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

				READSTR( _instance, _stream, resource->name );

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
					READPOLYGON( _instance, _stream, it_polygon );
				}

				*it_resource = (aeMovieResource *)resource;
			}break;
		case 3:
			{
				aeMovieResourceSocketImage * resource = NEW( _instance, aeMovieResourceSocketImage );

				READSTR( _instance, _stream, resource->path );

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

				READSTR( _instance, _stream, resource->path );

				READ( _stream, resource->alpha );
				READ( _stream, resource->frameRate );
				READ( _stream, resource->duration );

				*it_resource = (aeMovieResource *)resource;
			}break;
		case 6:
			{
				aeMovieResourceSound * resource = NEW( _instance, aeMovieResourceSound );

				READSTR( _instance, _stream, resource->path );

				READ( _stream, resource->duration );

				*it_resource = (aeMovieResource *)resource;
			}break;
		case 7:
			{
				aeMovieResourceImage * resource = NEW( _instance, aeMovieResourceImage );

				READSTR( _instance, _stream, resource->path );

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
		*compositionData = _movie->compositions,
		*compositionData_end = _movie->compositions + composition_count;
	compositionData != compositionData_end;
	++compositionData )
	{
		READSTR( _instance, _stream, compositionData->name );

		READ( _stream, compositionData->width );
		READ( _stream, compositionData->height );

		READ( _stream, compositionData->frameDuration );
		READ( _stream, compositionData->duration );

		compositionData->flags = 0;

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
					READN( _stream, compositionData->loopSegment, 2 );

					compositionData->flags |= AE_MOVIE_COMPOSITION_LOOP_SEGMENT;
				}break;
			case 2:
				{
					READN( _stream, compositionData->anchorPoint, 3 );

					compositionData->flags |= AE_MOVIE_COMPOSITION_ANCHOR_POINT;
				}break;
			case 3:
				{
					READN( _stream, compositionData->offsetPoint, 3 );

					compositionData->flags |= AE_MOVIE_COMPOSITION_OFFSET_POINT;
				}break;
			case 4:
				{
					READN( _stream, compositionData->bounds, 4 );

					compositionData->flags |= AE_MOVIE_COMPOSITION_BOUNDS;
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

		compositionData->layers = NEWN( _instance, aeMovieLayerData, layer_count );

		for( uint32_t layer_index = 0; layer_index != layer_count; ++layer_index )
		{
			aeMovieLayerData * layer = compositionData->layers + layer_index;

			//frame_count
		}
	}

	return AE_MOVIE_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////