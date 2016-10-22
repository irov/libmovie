#	ifndef MOVIE_STREAM_H_
#	define MOVIE_STREAM_H_

#	include "movie/movie_type.h"

#	include "movie_memory.h"
#	include "movie_struct.h"

#	include <stddef.h>
//////////////////////////////////////////////////////////////////////////
#	define READ(stream, value) ((*stream->read)(stream->data, &(value), sizeof(value)))
#	define READN(stream, ptr, n) ((*stream->read)(stream->data, ptr, sizeof(*ptr) * n))
#	define READB(stream) ae_magic_read_bool(stream)
#	define READZ(stream) ae_magic_read_size(stream)
//////////////////////////////////////////////////////////////////////////
#	define READ_STRING(instance, stream, ptr) (ae_magic_read_string(instance, stream, &ptr))
#	define READ_POLYGON(instance, stream, ptr) (ae_magic_read_polygon(instance, stream, ptr))
#	define READ_VIEWPORT(stream, ptr) (ae_magic_read_viewport(stream, ptr))
#	define READ_MESH(instance, stream, ptr) (ae_magic_read_mesh(instance, stream, ptr))
//////////////////////////////////////////////////////////////////////////
static ae_bool_t ae_magic_read_bool( const aeMovieStream * _stream )
{
	ae_bool_t value;
	READ( _stream, value );

	return value;
}
//////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////
static void ae_magic_read_string( const aeMovieInstance * _instance, const aeMovieStream * _stream, ae_string_t * _str )
{
	uint32_t size = READZ( _stream );

	ae_string_t str = NEWN( _instance, ae_char_t, size + 1 );
	READN( _stream, str, size );

	str[size] = '\0';

	*_str = str;
}
//////////////////////////////////////////////////////////////////////////
static void ae_magic_read_polygon( const aeMovieInstance * _instance, const aeMovieStream * _stream, aeMoviePolygon * _polygon )
{
	uint32_t point_count = READZ( _stream );
	
	_polygon->point_count = point_count;

	if( point_count == 0 )
	{		
		_polygon->points = AE_NULL;

		return;
	}

    ae_vector2_t * points = NEWN( _instance, ae_vector2_t, point_count );
	READN( _stream, points, point_count );

    _polygon->points = points;
}
//////////////////////////////////////////////////////////////////////////
static void ae_magic_read_viewport( const aeMovieStream * _stream, aeMovieViewport * _viewport )
{
	READ( _stream, _viewport->begin_x );
	READ( _stream, _viewport->begin_y );
	READ( _stream, _viewport->end_x );
	READ( _stream, _viewport->end_y );
}
//////////////////////////////////////////////////////////////////////////
static void ae_magic_read_mesh( const aeMovieInstance * _instance, const aeMovieStream * _stream, aeMovieMesh * _mesh )
{
	uint16_t vertex_count = READZ( _stream );

	if( vertex_count == 0 || vertex_count > AE_MOVIE_MAX_VERTICES )
	{
		_mesh->vertex_count = 0;
		_mesh->indices_count = 0;

		_mesh->positions = AE_NULL;
		_mesh->uvs = AE_NULL;
		_mesh->indices = AE_NULL;

		return;	
	}

	uint16_t indices_count = READZ( _stream );

	_mesh->vertex_count = vertex_count;
	_mesh->indices_count = indices_count;

    ae_vector2_t * positions = NEWN( _instance, ae_vector2_t, vertex_count );
	READN( _stream, positions, vertex_count );
    _mesh->positions = positions;

    ae_vector2_t * uvs = NEWN(_instance, ae_vector2_t, vertex_count);
    READN(_stream, uvs, vertex_count);
    _mesh->uvs = uvs;
	
    uint16_t * indices = NEWN( _instance, uint16_t, indices_count );
	READN( _stream, indices, indices_count );
    _mesh->indices = indices;
}
#	endif
