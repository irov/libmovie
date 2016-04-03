#	ifndef MOVIE_STREAM_H_
#	define MOVIE_STREAM_H_

#	include <movie/movie_type.h>
#	include <movie/movie_instance.h>

#	include "memory.h"

#	include <stddef.h>

#	define READ(stream, value) ((*stream->read)(stream->data, &value, sizeof(value)))
#	define READN(stream, ptr, n) ((*stream->read)(stream->data, ptr, sizeof(*ptr) * n))
#	define READB(stream) ae_magic_read_bool(stream)
#	define READZ(stream) ae_magic_read_size(stream)

#	define READ_STRING(instance, stream, ptr) (ae_magic_read_string(instance, stream, &ptr))
#	define READ_POLYGON(instance, stream, ptr) (ae_magic_read_polygon(instance, stream, ptr))
#	define READ_VIEWPORT(stream, ptr) (ae_magic_read_viewport(stream, ptr))
#	define READ_MESH(instance, stream, ptr) (ae_magic_read_mesh(instance, stream, ptr))
#	define READ_PROPERTY(instance, stream, ptr, count) (ae_magic_read_property(instance, stream, ptr, count))


static ae_bool_t ae_magic_read_bool( const aeMovieStream * _stream )
{
	ae_bool_t value;
	READ( _stream, value );

	return value;
}

static uint32_t ae_magic_read_size( const aeMovieStream * _stream )
{
	uint8_t size255;
	READ( _stream, size255 );

	if( size255 != 255 )
	{
		return (uint32_t)size255;
	}

	uint16_t size65535;
	READ( _stream, size65535 );

	if( size65535 != 65535 )
	{
		return (uint32_t)size65535;
	}

	uint32_t size;
	READ( _stream, size );

	return size;
}

static void ae_magic_read_string( const aeMovieInstance * _instance, const aeMovieStream * _stream, char ** _str )
{
	uint32_t size = READZ( _stream );

	char * str = NEWN( _instance, char, size + 1 );
	READN( _stream, str, size );

	str[size] = '\0';

	*_str = str;
}

static void ae_magic_read_polygon( const aeMovieInstance * _instance, const aeMovieStream * _stream, aeMoviePolygon * _polygon )
{
	uint32_t point_count = READZ( _stream );
	
	_polygon->point_count = point_count;

	if( point_count == 0 )
	{		
		_polygon->points = AE_NULL;

		return;
	}

	_polygon->points = NEWN( _instance, float, point_count * 2 );
	READN( _stream, _polygon->points, point_count * 2 );
}

static void ae_magic_read_viewport( const aeMovieStream * _stream, aeMovieViewport * _viewport )
{
	READ( _stream, _viewport->begin_x );
	READ( _stream, _viewport->begin_y );
	READ( _stream, _viewport->end_x );
	READ( _stream, _viewport->end_y );
}

static void ae_magic_read_mesh( const aeMovieInstance * _instance, const aeMovieStream * _stream, aeMovieMesh * _mesh )
{
	uint16_t vertex_count = READZ( _stream );

	if( vertex_count == 0 )
	{
		_mesh->vertices = AE_NULL;
		_mesh->indices = AE_NULL;

		return;	
	}

	uint16_t indices_count = READZ( _stream );

	_mesh->vertex_count = vertex_count;
	_mesh->indices_count = indices_count;

	_mesh->vertices = NEWN( _instance, float, vertex_count * 2 );
	READN( _stream, _mesh->vertices, vertex_count * 2 );

	_mesh->indices = NEWN( _instance, uint16_t, indices_count );
	READN( _stream, _mesh->indices, indices_count );
}

static void ae_magic_read_zp( const aeMovieStream * _stream, float * _values )
{
	uint32_t count = READZ( _stream );

	float * stream_values = _values;

	for( uint32_t i = 0; i != count; ++i )
	{
		uint8_t block_type;
		READ( _stream, block_type );

		uint8_t block_count;
		READ( _stream, block_count );

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

static void ae_magic_read_property( const aeMovieInstance * _instance, const aeMovieStream * _stream, aeMovieLayerProperty * _property, uint32_t _count )
{
	uint16_t immutable_mask;
	READ( _stream, immutable_mask );

	_property->immutable_mask = immutable_mask;

#	define AE_MOVIE_STREAM_PROPERTY(Mask, ImmutableName, Name)\
	if( immutable_mask & Mask )\
	{\
		READ( _stream, _property->ImmutableName );\
		_property->Name = AE_NULL;\
	}\
	else\
	{\
		_property->ImmutableName = 0.f;\
		_property->Name = NEWN( _instance, float, _count );\
		ae_magic_read_zp( _stream, _property->Name );\
	}

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_X, immuttable_anchor_point_x, anchor_point_x );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Y, immuttable_anchor_point_y, anchor_point_y );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Z, immuttable_anchor_point_z, anchor_point_z );

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_X, immuttable_position_x, position_x );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_Y, immuttable_position_x, position_y );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_Z, immuttable_position_x, position_z );

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ROTATION_X, immuttable_rotation_x, rotation_x );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ROTATION_Y, immuttable_rotation_y, rotation_y );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ROTATION_Z, immuttable_rotation_z, rotation_z );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ROTATION_W, immuttable_rotation_w, rotation_w );

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_X, immuttable_scale_x, scale_x );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_Y, immuttable_scale_y, scale_y );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_Z, immuttable_scale_z, scale_z );

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_OPACITY, immuttable_opacity, opacity );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_VOLUME, immuttable_volume, volume );

#	undef AE_MOVIE_STREAM_PROPERTY
}

#	endif
