#	ifndef MOVIE_STREAM_H_
#	define MOVIE_STREAM_H_

#	include "movie/movie_type.h"

#	include "movie_memory.h"
#	include "movie_struct.h"

#	include <stddef.h>
//////////////////////////////////////////////////////////////////////////
#	ifdef AE_MOVIE_STREAM_INFO
#	define READ(stream, value) ae_magic_read_value_info(stream, #value, &(value), sizeof(value))
#	define READN(stream, ptr, n) ae_magic_read_value_info(stream, #ptr, ptr, sizeof(*ptr) * n)
#	else
#	define READ(stream, value) ae_magic_read_value(stream, &(value), sizeof(value))
#	define READN(stream, ptr, n) ae_magic_read_value(stream, ptr, sizeof(*ptr) * n)
#	endif
//////////////////////////////////////////////////////////////////////////
#	define READB(stream) ae_magic_read_bool(stream)
#	define READZ(stream) ae_magic_read_size(stream)
//////////////////////////////////////////////////////////////////////////
#	define READ_STRING(instance, stream, ptr) (ae_magic_read_string(instance, stream, &ptr))
#	define READ_POLYGON(instance, stream, ptr) (ae_magic_read_polygon(instance, stream, ptr))
#	define READ_VIEWPORT(stream, ptr) (ae_magic_read_viewport(stream, ptr))
#	define READ_MESH(instance, stream, ptr) (ae_magic_read_mesh(instance, stream, ptr))
//////////////////////////////////////////////////////////////////////////
#	ifdef AE_MOVIE_STREAM_INFO
//////////////////////////////////////////////////////////////////////////
static void ae_magic_read_value_info( aeMovieStream * _stream, const char * _info, void * _ptr, size_t _size )
{
	_stream->instance->logerror( _stream->instance->instance_data, AE_ERROR_STREAM, "read stream '%s' size '%d'", _info, (uint32_t)_size );

	size_t bytesRead = _stream->memory_read( _stream->data, _ptr, _size );

	(void)bytesRead;
}
#	endif
//////////////////////////////////////////////////////////////////////////
#	ifdef AE_MOVIE_STREAM_CACHE
static void ae_magic_read_value( aeMovieStream * _stream, void * _ptr, size_t _size )
{
	size_t carriage = _stream->carriage;
	size_t capacity = _stream->capacity;
	size_t reading = _stream->reading;

	if( _size > AE_MOVIE_STREAM_CACHE_BUFFER_SIZE )
	{
		size_t tail = capacity - carriage;

		if( tail != 0 )
		{
			const void * buff_carriage = _stream->buff + carriage;
			_stream->memory_copy( _stream->data, buff_carriage, _ptr, tail );
		}

		size_t correct_read = _size - tail;
		void * correct_ptr = (uint8_t *)_ptr + tail;

		size_t bytesRead = _stream->memory_read( _stream->data, correct_ptr, correct_read );

		_stream->carriage = 0;
		_stream->capacity = 0;

		_stream->reading += bytesRead;

		return;
	}

	if( carriage + _size <= capacity )
	{
		const void * buff_carriage = _stream->buff + carriage;

		_stream->memory_copy( _stream->data, buff_carriage, _ptr, _size );

		_stream->carriage += _size;

		return;
	}

	size_t tail = capacity - carriage;

	if( tail != 0 )
	{
		const void * buff_carriage = _stream->buff + carriage;

		_stream->memory_copy( _stream->data, buff_carriage, _ptr, tail );
	}

	size_t bytesRead = _stream->memory_read( _stream->data, _stream->buff, AE_MOVIE_STREAM_CACHE_BUFFER_SIZE );

	size_t readSize = _size - tail;

	if( readSize > bytesRead )
	{
		readSize = bytesRead;
	}
	
	_stream->memory_copy( _stream->data, _stream->buff, _ptr, readSize );

	_stream->carriage = readSize;
	_stream->capacity = bytesRead;

	_stream->reading += AE_MOVIE_STREAM_CACHE_BUFFER_SIZE;
}
#else
static void ae_magic_read_value(aeMovieStream * _stream, void * _ptr, size_t _size)
{
	size_t bytesRead = _stream->memory_read(_stream->data, _ptr, _size);
	
	(void)bytesRead;
}
#endif
//////////////////////////////////////////////////////////////////////////
static ae_bool_t ae_magic_read_bool( aeMovieStream * _stream )
{
	ae_bool_t value;
	READ( _stream, value );

	return value;
}
//////////////////////////////////////////////////////////////////////////
static uint32_t ae_magic_read_size( aeMovieStream * _stream )
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
static void ae_magic_read_string( const aeMovieInstance * _instance, aeMovieStream * _stream, ae_string_t * _str )
{
	uint32_t size = READZ( _stream );

	ae_string_t str = NEWN( _instance, ae_char_t, size + 1 );
	READN( _stream, str, size );

	str[size] = '\0';

	*_str = str;
}
//////////////////////////////////////////////////////////////////////////
static void ae_magic_read_polygon( const aeMovieInstance * _instance, aeMovieStream * _stream, aeMoviePolygon * _polygon )
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

	_polygon->points = (const ae_vector2_t *)points;
}
//////////////////////////////////////////////////////////////////////////
static void ae_magic_read_viewport( aeMovieStream * _stream, aeMovieViewport * _viewport )
{
	READ( _stream, _viewport->begin_x );
	READ( _stream, _viewport->begin_y );
	READ( _stream, _viewport->end_x );
	READ( _stream, _viewport->end_y );
}
//////////////////////////////////////////////////////////////////////////
static void ae_magic_read_mesh( const aeMovieInstance * _instance, aeMovieStream * _stream, aeMovieMesh * _mesh )
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
	_mesh->positions = (const ae_vector2_t *)positions;

    ae_vector2_t * uvs = NEWN(_instance, ae_vector2_t, vertex_count);
    READN(_stream, uvs, vertex_count);
	_mesh->uvs = (const ae_vector2_t *)uvs;
	
    uint16_t * indices = NEWN( _instance, uint16_t, indices_count );
	READN( _stream, indices, indices_count );
    _mesh->indices = indices;
}
#	endif
