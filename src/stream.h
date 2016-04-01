#	ifndef MOVIE_STREAM_H_
#	define MOVIE_STREAM_H_

#	include <movie/movie_type.h>
#	include <movie/movie_instance.h>

#	include "memory.h"

#	include <stddef.h>

#	define READ(stream, value) ((*stream->read)(stream->data, &value, sizeof(value)))
#	define READN(stream, ptr, n) ((*stream->read)(stream->data, ptr, sizeof(*ptr) * n))
#	define READZ(stream) ae_magic_read_size(stream)

#	define READSTR(instance, stream, ptr) (ae_magic_read_string(instance, stream, &ptr))
#	define READPOLYGON(instance, stream, ptr) (ae_magic_read_polygon(instance, stream, ptr))
#	define READMESH(instance, stream, ptr) (ae_magic_read_mesh(instance, stream, ptr))

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
	uint16_t point_count = READZ( _stream );
	
	_polygon->point_count = point_count;

	if( point_count == 0 )
	{		
		_polygon->points = NULL;

		return;
	}

	_polygon->points = NEWN( _instance, float, point_count * 2 );
	READN( _stream, _polygon->points, point_count * 2 );
}

static void ae_magic_read_mesh( const aeMovieInstance * _instance, const aeMovieStream * _stream, aeMovieMesh * _mesh )
{
	uint16_t vertex_count = READZ( _stream );
	uint16_t indices_count = READZ( _stream );

	_mesh->vertex_count = vertex_count;
	_mesh->indices_count = indices_count;

	if( vertex_count == 0 || indices_count == 0 )
	{
		_mesh->vertices = NULL;
		_mesh->indices = NULL;

		return;
	}

	_mesh->vertices = NEWN( _instance, float, vertex_count * 2 );
	READN( _stream, _mesh->vertices, vertex_count * 2 );

	_mesh->indices = NEWN( _instance, uint16_t, indices_count );
	READN( _stream, _mesh->indices, indices_count * 2 );
}


#	endif
