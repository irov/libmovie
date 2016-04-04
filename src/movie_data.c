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
static void __load_movie_data_layer_property_zp( const aeMovieStream * _stream, float * _values )
{
	uint32_t count = READZ( _stream );

	float * stream_values = _values;

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
					*stream_values++ = block_base + block_add * block_index;
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
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////
static void __load_movie_data_layer_property( const aeMovieInstance * _instance, const aeMovieStream * _stream, aeMovieLayerData * _layer, uint32_t _count )
{
	uint16_t immutable_property_mask;
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
		__load_movie_data_layer_property_zp( _stream, _layer->Name );\
			}

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_X, immuttable_anchor_point_x, property_anchor_point_x );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Y, immuttable_anchor_point_y, property_anchor_point_y );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Z, immuttable_anchor_point_z, property_anchor_point_z );

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_X, immuttable_position_x, property_position_x );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_Y, immuttable_position_x, property_position_y );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_Z, immuttable_position_x, property_position_z );

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ROTATION_X, immuttable_rotation_x, property_rotation_x );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ROTATION_Y, immuttable_rotation_y, property_rotation_y );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ROTATION_Z, immuttable_rotation_z, property_rotation_z );

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_X, immuttable_scale_x, property_scale_x );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_Y, immuttable_scale_y, property_scale_y );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_Z, immuttable_scale_z, property_scale_z );

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_OPACITY, immuttable_opacity, property_opacity );

#	undef AE_MOVIE_STREAM_PROPERTY
}
//////////////////////////////////////////////////////////////////////////
static aeMovieResult __load_movie_data_layer( const aeMovieInstance * _instance, const aeMovieStream * _stream, const aeMovieData * _movie, const aeMovieCompositionData * _composition, aeMovieLayerData * _layer )
{
	READ_STRING( _instance, _stream, _layer->name );

	_layer->index = READZ( _stream );
	_layer->type = READZ( _stream );

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

				if( _layer->timeremap->immutable == AE_TRUE )
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

	__load_movie_data_layer_property( _instance, _stream, _layer, _layer->frame_count );

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
		default:
			{
				return AE_MOVIE_FAILED;
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