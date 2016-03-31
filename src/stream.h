#	ifndef MOVIE_STREAM_H_
#	define MOVIE_STREAM_H_

#	include <movie/movie_type.h>
#	include <movie/movie_instance.h>

#	include "memory.h"

#	define READ(stream, value) ((*stream->read)(stream->data, &value, sizeof(value)))
#	define READN(stream, ptr, n) ((*stream->read)(stream->data, ptr, sizeof(*ptr) * n))
#	define READZ(stream) ae_magic_read_size(stream)
#	define READSTR(instance, stream, ptr) (ae_magic_read_string(instance, stream, &ptr))

static uint32_t ae_magic_read_size( const aeMovieStream * _stream )
{
	uint8_t size255;
	READ( _stream, size255 );

	if( size255 != 255 )
	{
		return (uint32_t)size255;
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

#	endif
