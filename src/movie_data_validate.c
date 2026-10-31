/******************************************************************************
* libMOVIE Software License v1.0
*
* Copyright (c) 2016-2026, Yuriy Levchenko <irov13@mail.ru>
* All rights reserved.
*
* You are granted a perpetual, non-exclusive, non-sublicensable, and
* non-transferable license to use, install, execute, and perform the libMOVIE
* software and derivative works solely for personal or internal
* use. Without the written permission of Yuriy Levchenko, you may not (a) modify, translate,
* adapt, or develop new applications using the libMOVIE or otherwise
* create derivative works or improvements of the libMOVIE or (b) remove,
* delete, alter, or obscure any trademarks or any copyright, trademark, patent,
* or other intellectual property or proprietary rights notices on or in the
* Software, including any copy thereof. Redistributions in binary or source
* form must include this license and terms.
*
* THIS SOFTWARE IS PROVIDED BY YURIY LEVCHENKO "AS IS" AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL YURIY LEVCHENKO BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES, BUSINESS INTERRUPTION,
* OR LOSS OF USE, DATA, OR PROFITS) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include "movie_data_validate.h"
#include "movie_stream.h"
#include "movie_struct.h"

#include <math.h>
#include <string.h>

#define DATA_COUNT_LIMIT 1048576U

typedef struct DataReader
{
    const ae_uint8_t * bytes;
    ae_size_t size, position;
    ae_bool_t failed;
    const aeMovieInstance * instance;
} DataReader;

//////////////////////////////////////////////////////////////////////////
static ae_void_t data_read( DataReader * _reader, ae_voidptr_t _output, ae_size_t _size )
{
    if( _size > _reader->size - _reader->position )
    {
        _reader->failed = AE_TRUE;
        memset( _output, 0, _size );

        return;
    }

    memcpy( _output, _reader->bytes + _reader->position, _size );
    _reader->position += _size;
}

//////////////////////////////////////////////////////////////////////////
static ae_uint32_t data_u32( DataReader * _reader )
{
    ae_uint8_t value[4];
    data_read( _reader, value, 4 );

    ae_uint32_t result = (ae_uint32_t)value[0] | ( (ae_uint32_t)value[1] << 8 ) | ( (ae_uint32_t)value[2] << 16 ) | ( (ae_uint32_t)value[3] << 24 );

    return result;
}

//////////////////////////////////////////////////////////////////////////
static ae_float_t data_float( DataReader * _reader )
{
    ae_uint32_t bits = data_u32( _reader );
    ae_float_t value;
    memcpy( &value, &bits, 4 );

    if( isfinite( value ) == 0 )
    {
        _reader->failed = AE_TRUE;
    }

    return value;
}

//////////////////////////////////////////////////////////////////////////
static ae_size_t data_stream_read( ae_voidptr_t _output, ae_size_t _position, ae_size_t _size, ae_userdata_t _userdata )
{
    DataReader * reader = (DataReader *)_userdata;
    reader->position = _position;

    if( _position > reader->size )
    {
        reader->position = reader->size;
        reader->failed = AE_TRUE;
    }

    data_read( reader, _output, _size );

    return _size;
}

//////////////////////////////////////////////////////////////////////////
static ae_void_t data_copy( ae_constvoidptr_t _source, ae_voidptr_t _destination, ae_size_t _size, ae_userdata_t _userdata )
{
    AE_UNUSED( _userdata );
    memcpy( _destination, _source, _size );
}

typedef struct DataGraphLayer
{
    ae_uint32_t child;
} DataGraphLayer;

typedef struct DataGraphCheck
{
    ae_uint32_t resource_type[8192];
    ae_uint32_t first[1024], count[1024], expanded[1024];
    ae_uint8_t visited[1024];
    DataGraphLayer layers[65536];
    ae_uint32_t used;
} DataGraphCheck;

//////////////////////////////////////////////////////////////////////////
static ae_uint32_t data_byte( DataReader * _reader )
{
    ae_uint8_t value;
    data_read( _reader, &value, 1 );

    return value;
}

//////////////////////////////////////////////////////////////////////////
static ae_uint32_t data_size( DataReader * _reader )
{
    ae_uint32_t value = data_byte( _reader );

    if( value == 255 )
    {
        ae_uint32_t low = data_byte( _reader );
        value = low | ( data_byte( _reader ) << 8 );

        if( value == 65535 )
        {
            value = data_u32( _reader );
        }
    }

    return value;
}

//////////////////////////////////////////////////////////////////////////
static ae_bool_t data_skip_string( DataReader * _reader )
{
    ae_uint32_t size = data_size( _reader );

    if( size > 4096 )
    {
        return AE_FALSE;
    }

    if( size > _reader->size - _reader->position )
    {
        return AE_FALSE;
    }

    if( memchr( _reader->bytes + _reader->position, 0, size ) != AE_NULLPTR )
    {
        return AE_FALSE;
    }

    _reader->position += size;

    return !_reader->failed;
}

//////////////////////////////////////////////////////////////////////////
static ae_bool_t data_floats( DataReader * _reader, ae_uint32_t _count )
{
    ae_uint32_t index;

    if( _count > ( _reader->size - _reader->position ) / 4 )
    {
        return AE_FALSE;
    }

    for( index = 0; index != _count; ++index )
    {
        data_float( _reader );
    }

    return !_reader->failed;
}

//////////////////////////////////////////////////////////////////////////
static ae_bool_t data_graph_visit( DataGraphCheck * _graph, ae_uint32_t _composition, ae_uint32_t _depth )
{
    ae_uint32_t index, expanded = _graph->count[_composition];

    if( _depth > 32 )
    {
        return AE_FALSE;
    }

    if( _graph->visited[_composition] == 1 )
    {
        return AE_FALSE;
    }

    if( _graph->visited[_composition] == 2 )
    {
        return AE_TRUE;
    }

    _graph->visited[_composition] = 1;

    for( index = 0; index != _graph->count[_composition]; ++index )
    {
        ae_uint32_t child = _graph->layers[_graph->first[_composition] + index].child;

        if( child == 0xffffffffU )
        {
            continue;
        }

        if( data_graph_visit( _graph, child, _depth + 1 ) == AE_FALSE )
        {
            return AE_FALSE;
        }

        if( _graph->expanded[child] > 65536 - expanded )
        {
            return AE_FALSE;
        }

        expanded += _graph->expanded[child];
    }

    _graph->expanded[_composition] = expanded;
    _graph->visited[_composition] = 2;

    return AE_TRUE;
}

//////////////////////////////////////////////////////////////////////////
static ae_bool_t data_validate_content_( DataReader * _reader, DataGraphCheck * _graph )
{
    ae_uint32_t resources, compositions, index, ci;
    _reader->position = 0;

    if( data_skip_string( _reader ) == AE_FALSE )
    {
        return AE_FALSE;
    }

    if( data_byte( _reader ) != 0 )
    {
        return AE_FALSE;
    }

    if( data_size( _reader ) != 0 )
    {
        return AE_FALSE;
    }

    resources = data_size( _reader );

    if( resources > 8192 )
    {
        return AE_FALSE;
    }

    for( index = 0; index != resources; ++index )
    {
        ae_uint32_t type = data_byte( _reader ), count, n;
        _graph->resource_type[index] = type;

        if( data_skip_string( _reader ) == AE_FALSE )
        {
            return AE_FALSE;
        }

        if( type == AE_MOVIE_RESOURCE_IMAGE || type == AE_MOVIE_RESOURCE_SOUND || type == AE_MOVIE_RESOURCE_PARTICLE || type == AE_MOVIE_RESOURCE_VIDEO )
        {
            if( data_skip_string( _reader ) == AE_FALSE )
            {
                return AE_FALSE;
            }

            if( data_byte( _reader ) != 0 )
            {
                return AE_FALSE;
            }
        }

        switch( type )
        {
        case AE_MOVIE_RESOURCE_IMAGE:
            {
                if( data_u32( _reader ) != 0 )
                {
                    return AE_FALSE;
                }

                if( data_floats( _reader, 2 ) == AE_FALSE )
                {
                    return AE_FALSE;
                }

                for( n = 0; n != 3; ++n )
                {
                    ae_uint32_t flag = data_byte( _reader );

                    if( flag == 0 )
                    {
                        break;
                    }

                    if( flag != 1 && flag != 2 )
                    {
                        return AE_FALSE;
                    }

                    if( data_floats( _reader, flag == 1 ? 4 : 8 ) == AE_FALSE )
                    {
                        return AE_FALSE;
                    }
                }

                if( n == 3 )
                {
                    return AE_FALSE;
                }
            }break;
        case AE_MOVIE_RESOURCE_SOUND:
            {
                if( data_float( _reader ) <= 0 )
                {
                    return AE_FALSE;
                }

                if( data_byte( _reader ) != 0 )
                {
                    return AE_FALSE;
                }
            }break;
        case AE_MOVIE_RESOURCE_SOLID:
            {
                if( data_floats( _reader, 5 ) == AE_FALSE )
                {
                    return AE_FALSE;
                }

                if( data_byte( _reader ) != 0 )
                {
                    return AE_FALSE;
                }
            }break;
        case AE_MOVIE_RESOURCE_VIDEO:
            {
                if( data_floats( _reader, 2 ) == AE_FALSE )
                {
                    return AE_FALSE;
                }

                if( data_byte( _reader ) > 1 )
                {
                    return AE_FALSE;
                }

                if( data_floats( _reader, 2 ) == AE_FALSE )
                {
                    return AE_FALSE;
                }

                if( data_byte( _reader ) != 0 )
                {
                    return AE_FALSE;
                }
            }break;
        case AE_MOVIE_RESOURCE_SEQUENCE:
        case AE_MOVIE_RESOURCE_PARTICLE:
            {
                if( type == AE_MOVIE_RESOURCE_SEQUENCE && data_float( _reader ) <= 0.f )
                {
                    return AE_FALSE;
                }

                count = data_size( _reader );

                if( count > 8192 )
                {
                    return AE_FALSE;
                }

                for( n = 0; n != count; ++n )
                {
                    ae_uint32_t image = data_size( _reader );

                    if( image >= index )
                    {
                        return AE_FALSE;
                    }

                    if( _graph->resource_type[image] != AE_MOVIE_RESOURCE_IMAGE )
                    {
                        return AE_FALSE;
                    }
                }

                if( data_byte( _reader ) != 0 )
                {
                    return AE_FALSE;
                }
            }break;
        default:
            {
                return AE_FALSE;
            }break;
        }
    }

    compositions = data_size( _reader );

    if( compositions == 0 )
    {
        return AE_FALSE;
    }

    if( compositions > 1024 )
    {
        return AE_FALSE;
    }

    for( ci = 0; ci != compositions; ++ci )
    {
        ae_uint32_t parents[4096], count, li;
        ae_float_t duration, fps;

        if( data_skip_string( _reader ) == AE_FALSE )
        {
            return AE_FALSE;
        }

        if( data_byte( _reader ) != 1 )
        {
            return AE_FALSE;
        }

        if( data_floats( _reader, 2 ) == AE_FALSE )
        {
            return AE_FALSE;
        }

        duration = data_float( _reader );

        if( duration <= 0 )
        {
            return AE_FALSE;
        }

        if( duration > 3600 )
        {
            return AE_FALSE;
        }

        if( data_float( _reader ) <= 0 )
        {
            return AE_FALSE;
        }

        fps = data_float( _reader );

        if( _reader->failed == AE_TRUE )
        {
            return AE_FALSE;
        }

        if( fps < 1 )
        {
            return AE_FALSE;
        }

        if( fps > 240 )
        {
            return AE_FALSE;
        }

        if( data_byte( _reader ) != 4 )
        {
            return AE_FALSE;
        }

        if( data_floats( _reader, 4 ) == AE_FALSE )
        {
            return AE_FALSE;
        }

        if( data_byte( _reader ) != 0 )
        {
            return AE_FALSE;
        }

        count = data_size( _reader );

        if( count > 4096 )
        {
            return AE_FALSE;
        }

        if( count > 65536 - _graph->used )
        {
            return AE_FALSE;
        }

        _graph->first[ci] = _graph->used;
        _graph->count[ci] = count;

        for( li = 0; li != count; ++li )
        {
            ae_uint32_t type, frames, flag, reference, is_resource, geometry;

            if( data_skip_string( _reader ) == AE_FALSE )
            {
                return AE_FALSE;
            }

            if( data_size( _reader ) != li + 1 )
            {
                return AE_FALSE;
            }

            if( data_byte( _reader ) > 1 )
            {
                return AE_FALSE;
            }

            flag = data_byte( _reader );

            if( flag > 1 )
            {
                return AE_FALSE;
            }

            if( flag != 0 && ( li + 1 == count || data_byte( _reader ) > AE_MOVIE_TRACK_MATTE_LUMA_INVERTED ) )
            {
                return AE_FALSE;
            }

            type = data_byte( _reader );
            frames = data_size( _reader );

            if( frames == 0 )
            {
                return AE_FALSE;
            }

            if( frames > (ae_uint32_t)ceilf( duration * fps ) + 2 )
            {
                return AE_FALSE;
            }

            geometry = data_byte( _reader );

            if( geometry != 0 )
            {
                ae_uint32_t immutable = data_byte( _reader ), frame;

                if( ( geometry != 2 && geometry != 3 ) )
                {
                    return AE_FALSE;
                }

                if( immutable > 1 )
                {
                    return AE_FALSE;
                }

                for( frame = 0; frame != ( immutable ? 1 : frames ); ++frame )
                {
                    if( geometry == 2 )
                    {
                        ae_uint32_t vertices = data_size( _reader ), indices = data_size( _reader ), element;

                        if( vertices < 3 )
                        {
                            return AE_FALSE;
                        }

                        if( vertices > AE_MOVIE_MAX_VERTICES )
                        {
                            return AE_FALSE;
                        }

                        if( indices == 0 )
                        {
                            return AE_FALSE;
                        }

                        if( indices > 65535 )
                        {
                            return AE_FALSE;
                        }

                        if( indices % 3 != 0 )
                        {
                            return AE_FALSE;
                        }

                        if( data_floats( _reader, vertices * 4 ) == AE_FALSE )
                        {
                            return AE_FALSE;
                        }

                        for( element = 0; element != indices; ++element )
                        {
                            ae_uint32_t low = data_byte( _reader ), vertex = low | ( data_byte( _reader ) << 8 );

                            if( vertex >= vertices )
                            {
                                return AE_FALSE;
                            }
                        }
                    }
                    else if( data_floats( _reader, 24 ) == AE_FALSE )
                    {
                        return AE_FALSE;
                    }
                }

                if( geometry == 3 && data_byte( _reader ) >= AE_MOVIE_BEZIER_MAX_QUALITY )
                {
                    return AE_FALSE;
                }

                if( data_byte( _reader ) != 0 )
                {
                    return AE_FALSE;
                }

                if( data_byte( _reader ) != 0 )
                {
                    return AE_FALSE;
                }
            }

            is_resource = data_byte( _reader );
            reference = data_size( _reader );
            _graph->layers[_graph->used].child = 0xffffffffU;

            if( is_resource == 1 )
            {
                ae_uint32_t expected = AE_MOVIE_LAYER_TYPE_NULL;

                if( reference > resources )
                {
                    return AE_FALSE;
                }

                if( reference != 0 )
                {
                    switch( _graph->resource_type[reference - 1] )
                    {
                    case AE_MOVIE_RESOURCE_IMAGE:
                        {
                            expected = AE_MOVIE_LAYER_TYPE_IMAGE;
                        }break;
                    case AE_MOVIE_RESOURCE_SOUND:
                        {
                            expected = AE_MOVIE_LAYER_TYPE_SOUND;
                        }break;
                    case AE_MOVIE_RESOURCE_SOLID:
                        {
                            expected = AE_MOVIE_LAYER_TYPE_SOLID;
                        }break;
                    case AE_MOVIE_RESOURCE_VIDEO:
                        {
                            expected = AE_MOVIE_LAYER_TYPE_VIDEO;
                        }break;
                    case AE_MOVIE_RESOURCE_PARTICLE:
                        {
                            expected = AE_MOVIE_LAYER_TYPE_PARTICLE;
                        }break;
                    case AE_MOVIE_RESOURCE_SEQUENCE:
                        {
                            expected = AE_MOVIE_LAYER_TYPE_SEQUENCE;
                        }break;
                    }
                }

                if( type != expected && !( reference == 0 && type == AE_MOVIE_LAYER_TYPE_SLOT ) )
                {
                    return AE_FALSE;
                }
            }
            else
            {
                if( is_resource != 0 )
                {
                    return AE_FALSE;
                }

                if( reference >= compositions )
                {
                    return AE_FALSE;
                }

                if( type != AE_MOVIE_LAYER_TYPE_MOVIE )
                {
                    return AE_FALSE;
                }

                _graph->layers[_graph->used].child = reference;
            }

            if( geometry != 0 && type != AE_MOVIE_LAYER_TYPE_IMAGE )
            {
                return AE_FALSE;
            }

            ++_graph->used;
            parents[li] = data_size( _reader );

            if( parents[li] > count )
            {
                return AE_FALSE;
            }
            {
                ae_float_t begin = data_float( _reader ), end = data_float( _reader );
                ae_float_t start = data_float( _reader ), finish = data_float( _reader );

                if( _reader->failed == AE_TRUE )
                {
                    return AE_FALSE;
                }

                if( begin < 0 )
                {
                    return AE_FALSE;
                }

                if( end <= begin )
                {
                    return AE_FALSE;
                }

                if( end > duration )
                {
                    return AE_FALSE;
                }

                if( start != 0 )
                {
                    return AE_FALSE;
                }

                if( finish != duration )
                {
                    return AE_FALSE;
                }
            }

            if( data_byte( _reader ) != 0 )
            {
                return AE_FALSE;
            }

            if( data_byte( _reader ) != 0 )
            {
                return AE_FALSE;
            }

            flag = data_byte( _reader );

            if( flag != AE_MOVIE_BLEND_NORMAL && flag != AE_MOVIE_BLEND_ADD && flag != AE_MOVIE_BLEND_SCREEN && flag != AE_MOVIE_BLEND_MULTIPLY )
            {
                return AE_FALSE;
            }

            if( data_byte( _reader ) != 0 )
            {
                return AE_FALSE;
            }

            flag = data_u32( _reader );

            if( flag != 0 && ( flag != AE_OPTION( 'l', 'o', 'o', 'p' ) || data_u32( _reader ) != 0 ) )
            {
                return AE_FALSE;
            }

            if( data_size( _reader ) != 1 )
            {
                return AE_FALSE;
            }

            if( data_float( _reader ) != 1 )
            {
                return AE_FALSE;
            }

            if( data_u32( _reader ) != 0xffffffffU )
            {
                return AE_FALSE;
            }

            if( data_u32( _reader ) != 0xffffffffU )
            {
                return AE_FALSE;
            }

            if( _reader->failed == AE_TRUE )
            {
                return AE_FALSE;
            }
        }

        for( li = 0; li != count; ++li )
        {
            ae_uint32_t current = li + 1, depth;

            for( depth = 0; current != 0; ++depth )
            {
                if( depth == count )
                {
                    return AE_FALSE;
                }

                current = parents[current - 1];
            }
        }
    }

    for( ci = 0; ci != compositions; ++ci )
    {
        if( data_graph_visit( _graph, ci, 0 ) == AE_FALSE )
        {
            return AE_FALSE;
        }
    }

    if( _reader->failed == AE_TRUE )
    {
        return AE_FALSE;
    }

    if( _reader->position != _reader->size )
    {
        return AE_FALSE;
    }

    return AE_TRUE;
}

//////////////////////////////////////////////////////////////////////////
static ae_bool_t data_validate_content( DataReader * _reader )
{
    DataGraphCheck * graph = AE_NEW( _reader->instance, DataGraphCheck );
    ae_bool_t result;

    if( graph == AE_NULLPTR )
    {
        return AE_FALSE;
    }

    memset( graph, 0, sizeof( *graph ) );
    result = data_validate_content_( _reader, graph );
    AE_DELETE( _reader->instance, graph );

    return result;
}

//////////////////////////////////////////////////////////////////////////
ae_result_t ae_movie_load_data_content( aeMovieData * _data, aeMovieStream * _stream, ae_movie_load_data_content_t _load )
{
    const aeMovieInstance * instance = _data->instance;
    ae_uint32_t size = 0;
    ae_uint32_t checksum = 0;
    AE_READ( _stream, size );
    AE_READ( _stream, checksum );

    if( size == 0 || size > 67108864U )
    {
        return AE_RESULT_INVALID_DATA;
    }

    ae_uint8_t * payload = AE_NEWN( instance, ae_uint8_t, size );

    if( payload == AE_NULLPTR )
    {
        return AE_RESULT_INVALID_MEMORY;
    }

    ae_result_t result = AE_RESULT_INVALID_STREAM;

    if( _stream->memory_read( payload, _stream->carriage, size, _stream->read_userdata ) != size )
    {
        AE_DELETEN( instance, payload );

        return result;
    }

    _stream->carriage += size;
    ae_uint32_t hash = 2166136261U;

    for( ae_uint32_t index = 0; index != size; ++index )
    {
        hash = ( hash ^ payload[index] ) * 16777619U;
    }

    DataReader reader = { payload, size, 0, AE_FALSE, instance };

    if( hash != checksum || data_validate_content( &reader ) == AE_FALSE )
    {
        AE_DELETEN( instance, payload );

        return AE_RESULT_INVALID_DATA;
    }

    reader.position = 0;
    aeMovieStream * stream = ae_create_movie_stream( instance, data_stream_read, data_copy, &reader );

    if( stream == AE_NULLPTR )
    {
        AE_DELETEN( instance, payload );

        return AE_RESULT_INVALID_MEMORY;
    }

    result = ( *_load )( _data, stream );
    ae_delete_movie_stream( stream );
    AE_DELETEN( instance, payload );

    return result;
}
