/******************************************************************************
* libMOVIE Software License v1.0
*
* Copyright (c) 2016-2017, Levchenko Yuriy <irov13@mail.ru>
* All rights reserved.
*
* You are granted a perpetual, non-exclusive, non-sublicensable, and
* non-transferable license to use, install, execute, and perform the libMOVIE
* software and derivative works solely for personal or internal
* use. Without the written permission of Levchenko Yuriy, you may not (a) modify, translate,
* adapt, or develop new applications using the libMOVIE or otherwise
* create derivative works or improvements of the libMOVIE or (b) remove,
* delete, alter, or obscure any trademarks or any copyright, trademark, patent,
* or other intellectual property or proprietary rights notices on or in the
* Software, including any copy thereof. Redistributions in binary or source
* form must include this license and terms.
*
* THIS SOFTWARE IS PROVIDED BY LEVCHENKO YURIY "AS IS" AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL LEVCHENKO YURIY BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES, BUSINESS INTERRUPTION,
* OR LOSS OF USE, DATA, OR PROFITS) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#	ifndef MOVIE_STREAM_H_
#	define MOVIE_STREAM_H_

#	include "movie/movie_type.h"

#	include "movie_memory.h"
#	include "movie_struct.h"

#	include <stddef.h>
//////////////////////////////////////////////////////////////////////////
#	ifdef AE_MOVIE_STREAM_INFO
#	define READ(stream, value) ae_magic_read_value_info(stream, #value, &(value), sizeof(value))
#	define READV(stream, value, size) ae_magic_read_value_info(stream, #value, value, size)
#	define READN(stream, ptr, n) ae_magic_read_value_info(stream, #ptr, ptr, sizeof(*ptr) * n)
#	else
#	define READ(stream, value) ae_magic_read_value(stream, &(value), sizeof(value))
#	define READV(stream, value, size) ae_magic_read_value(stream, value, size)
#	define READN(stream, ptr, n) ae_magic_read_value(stream, ptr, sizeof(*ptr) * n)
#	endif
//////////////////////////////////////////////////////////////////////////
#	define READB(stream) ae_magic_read_bool(stream)
#	define READZ(stream) ae_magic_read_size(stream)
//////////////////////////////////////////////////////////////////////////
#	define READ_STRING(stream, ptr) (ae_magic_read_string(stream, &ptr))
#	define READ_POLYGON(stream, ptr) (ae_magic_read_polygon(stream, ptr))
#	define READ_VIEWPORT(stream, ptr) (ae_magic_read_viewport(stream, ptr))
#	define READ_MESH(stream, ptr) (ae_magic_read_mesh(stream, ptr))
//////////////////////////////////////////////////////////////////////////
#	ifdef AE_MOVIE_STREAM_INFO
//////////////////////////////////////////////////////////////////////////
static ae_size_t ae_magic_read_value_info( aeMovieStream * _stream, const ae_char_t * _info, ae_voidptr_t _ptr, ae_size_t _size )
{
    _stream->instance->logger( _stream->instance->instance_data, AE_ERROR_STREAM, "read stream '%s' size '%d'\n", _info, (ae_uint32_t)_size );

    ae_size_t bytesRead = _stream->memory_read( _stream->data, _ptr, _size );

    return bytesRead;
}
//////////////////////////////////////////////////////////////////////////
#	endif
//////////////////////////////////////////////////////////////////////////
#	ifdef AE_MOVIE_STREAM_CACHE
//////////////////////////////////////////////////////////////////////////
static ae_size_t ae_magic_read_value( aeMovieStream * _stream, ae_voidptr_t _ptr, ae_size_t _size )
{
    ae_size_t carriage = _stream->carriage;
    ae_size_t capacity = _stream->capacity;

    if( _size > AE_MOVIE_STREAM_CACHE_BUFFER_SIZE )
    {
        ae_size_t tail = capacity - carriage;

        if( tail != 0 )
        {
            ae_constvoidptr_t buff_carriage = _stream->buff + carriage;
            _stream->memory_copy( _stream->data, buff_carriage, _ptr, tail );
        }

        ae_size_t correct_read = _size - tail;
        ae_voidptr_t correct_ptr = (ae_uint8_t *)_ptr + tail;

        ae_size_t bytesRead = _stream->memory_read( _stream->data, correct_ptr, correct_read );

        _stream->carriage = 0;
        _stream->capacity = 0;

        _stream->reading += bytesRead;

        return bytesRead + tail;
    }

    if( carriage + _size <= capacity )
    {
        ae_constvoidptr_t buff_carriage = _stream->buff + carriage;

        _stream->memory_copy( _stream->data, buff_carriage, _ptr, _size );

        _stream->carriage += _size;

        return _size;
    }

    ae_size_t tail = capacity - carriage;

    if( tail != 0 )
    {
        ae_constvoidptr_t buff_carriage = _stream->buff + carriage;

        _stream->memory_copy( _stream->data, buff_carriage, _ptr, tail );
    }

    ae_size_t bytesRead = _stream->memory_read( _stream->data, _stream->buff, AE_MOVIE_STREAM_CACHE_BUFFER_SIZE );

    ae_size_t readSize = _size - tail;

    if( readSize > bytesRead )
    {
        readSize = bytesRead;
    }

    _stream->memory_copy( _stream->data, _stream->buff, _ptr, readSize );

    _stream->carriage = readSize;
    _stream->capacity = bytesRead;

    _stream->reading += AE_MOVIE_STREAM_CACHE_BUFFER_SIZE;

    return bytesRead + tail;
}
//////////////////////////////////////////////////////////////////////////
#else
//////////////////////////////////////////////////////////////////////////
static ae_size_t ae_magic_read_value( aeMovieStream * _stream, ae_voidptr_t _ptr, ae_size_t _size )
{
    ae_size_t bytesRead = _stream->memory_read( _stream->data, _ptr, _size );

    return bytesRead;
}
//////////////////////////////////////////////////////////////////////////
#endif
//////////////////////////////////////////////////////////////////////////
static ae_bool_t ae_magic_read_bool( aeMovieStream * _stream )
{
    ae_uint8_t value;
    READ( _stream, value );

    return value;
}
//////////////////////////////////////////////////////////////////////////
static ae_uint32_t ae_magic_read_size( aeMovieStream * _stream )
{
    ae_uint8_t size255;
    READ( _stream, size255 );

    if( size255 != 255 )
    {
        return (ae_uint32_t)size255;
    }

    ae_uint16_t size65535;
    READ( _stream, size65535 );

    if( size65535 != 65535 )
    {
        return (ae_uint32_t)size65535;
    }

    ae_uint32_t size;
    READ( _stream, size );

    return size;
}
//////////////////////////////////////////////////////////////////////////
static void ae_magic_read_string( aeMovieStream * _stream, ae_string_t * _str )
{
    ae_uint32_t size = READZ( _stream );

    ae_string_t str = AE_NEWN( _stream->instance, ae_char_t, size + 1U );
    READN( _stream, str, size );

    str[size] = '\0';

    *_str = str;
}
//////////////////////////////////////////////////////////////////////////
static void ae_magic_read_polygon( aeMovieStream * _stream, aeMoviePolygon * _polygon )
{
    ae_uint32_t point_count = READZ( _stream );

    _polygon->point_count = point_count;

    if( point_count == 0 )
    {
        _polygon->points = AE_NULL;

        return;
    }

    ae_vector2_t * points = AE_NEWN( _stream->instance, ae_vector2_t, point_count );
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
static void ae_magic_read_mesh( aeMovieStream * _stream, aeMovieMesh * _mesh )
{
    ae_uint32_t vertex_count = READZ( _stream );

    if( vertex_count == 0 || vertex_count > AE_MOVIE_MAX_VERTICES )
    {
        _mesh->vertex_count = 0;
        _mesh->indices_count = 0;

        _mesh->positions = AE_NULL;
        _mesh->uvs = AE_NULL;
        _mesh->indices = AE_NULL;

        return;
    }

    ae_uint32_t indices_count = READZ( _stream );

    _mesh->vertex_count = vertex_count;
    _mesh->indices_count = indices_count;

    ae_vector2_t * positions = AE_NEWN( _stream->instance, ae_vector2_t, vertex_count );
    READN( _stream, positions, vertex_count );
    _mesh->positions = (const ae_vector2_t *)positions;

    ae_vector2_t * uvs = AE_NEWN( _stream->instance, ae_vector2_t, vertex_count );
    READN( _stream, uvs, vertex_count );
    _mesh->uvs = (const ae_vector2_t *)uvs;

    ae_uint16_t * indices = AE_NEWN( _stream->instance, ae_uint16_t, indices_count );
    READN( _stream, indices, indices_count );
    _mesh->indices = indices;
}
//////////////////////////////////////////////////////////////////////////
#	endif
