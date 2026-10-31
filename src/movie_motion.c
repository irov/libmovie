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

#include "movie_motion.h"
#include "movie_stream.h"
#include "movie_math.h"
#include "movie_transformation.h"
#include "movie/movie_resource.h"

#include <math.h>
#include <string.h>
#include <stdint.h>

#define MOTION_LIMIT 1048576U
#define MOTION_STACK 128U

typedef struct MotionReader
{
    const ae_uint8_t * bytes;
    ae_size_t size, position;
    ae_bool_t failed;
    const aeMovieInstance * instance;
} MotionReader;

//////////////////////////////////////////////////////////////////////////
typedef struct aeMotionParameterDefault
{
    const ae_char_t * name;
    ae_float_t value;
} aeMotionParameterDefault;

//////////////////////////////////////////////////////////////////////////
static const aeMotionParameterDefault MOTION_PARAMETER_DEFAULTS[AE_MOVIE_MOTION_PARAMETER_COUNT] = {
    { "speed", 1.f },
    { "intensity", 1.f },
    { "amplitude", 1.f },
    { "frequency", 1.f },
    { "phase", 0.f },
    { "progress", 0.f },
    { "blend", 0.f },
    { "directionX", 1.f },
    { "directionY", 0.f },
    { "targetX", 0.f },
    { "targetY", 0.f },
    { "damping", 1.f },
    { "seed", 0.f },
};

//////////////////////////////////////////////////////////////////////////
const ae_char_t * ae_get_movie_motion_parameter_name( aeMovieMotionParameter _parameter )
{
    if( (ae_uint32_t)_parameter >= AE_MOVIE_MOTION_PARAMETER_COUNT )
    {
        return AE_NULLPTR;
    }

    return MOTION_PARAMETER_DEFAULTS[_parameter].name;
}

//////////////////////////////////////////////////////////////////////////
ae_float_t ae_get_movie_motion_parameter_default( aeMovieMotionParameter _parameter )
{
    if( (ae_uint32_t)_parameter >= AE_MOVIE_MOTION_PARAMETER_COUNT )
    {
        return 0.f;
    }

    return MOTION_PARAMETER_DEFAULTS[_parameter].value;
}

//////////////////////////////////////////////////////////////////////////
static ae_uint32_t motion_parameter_slot( const ae_char_t * _name )
{
    ae_uint32_t index;

    for( index = 0; index != AE_MOVIE_MOTION_PARAMETER_COUNT; ++index )
    {
        if( strcmp( MOTION_PARAMETER_DEFAULTS[index].name, _name ) == 0 )
        {
            return index;
        }
    }

    return AE_MOVIE_MOTION_PARAMETER_COUNT;
}

//////////////////////////////////////////////////////////////////////////
static ae_void_t motion_read( MotionReader * _reader, ae_voidptr_t _output, ae_size_t _size )
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
static ae_uint32_t motion_u32( MotionReader * _reader )
{
    ae_uint8_t value[4];
    motion_read( _reader, value, 4 );

    ae_uint32_t result = (ae_uint32_t)value[0] | ( (ae_uint32_t)value[1] << 8 ) | ( (ae_uint32_t)value[2] << 16 ) | ( (ae_uint32_t)value[3] << 24 );

    return result;
}

//////////////////////////////////////////////////////////////////////////
static ae_float_t motion_float( MotionReader * _reader )
{
    ae_uint32_t bits = motion_u32( _reader );
    ae_float_t value;
    memcpy( &value, &bits, 4 );

    if( isfinite( value ) == 0 )
    {
        _reader->failed = AE_TRUE;
    }

    return value;
}

//////////////////////////////////////////////////////////////////////////
static ae_char_t * motion_string( MotionReader * _reader )
{
    ae_uint32_t size = motion_u32( _reader );
    ae_char_t * value;

    if( size > 4096 )
    {
        _reader->failed = AE_TRUE;

        return AE_NULLPTR;
    }

    if( size > _reader->size - _reader->position )
    {
        _reader->failed = AE_TRUE;

        return AE_NULLPTR;
    }

    value = AE_NEWN( _reader->instance, ae_char_t, size + 1 );

    if( value == AE_NULLPTR )
    {
        _reader->failed = AE_TRUE;

        return AE_NULLPTR;
    }

    motion_read( _reader, value, size );
    value[size] = '\0';

    return value;
}

//////////////////////////////////////////////////////////////////////////
static ae_voidptr_t motion_array( MotionReader * _reader, ae_uint32_t _count, ae_size_t _size )
{
    ae_voidptr_t value;

    if( _count > MOTION_LIMIT )
    {
        _reader->failed = AE_TRUE;

        return AE_NULLPTR;
    }

    if( _count > _reader->size )
    {
        _reader->failed = AE_TRUE;

        return AE_NULLPTR;
    }

    if( _count != 0 && _size > 67108864U / _count )
    {
        _reader->failed = AE_TRUE;

        return AE_NULLPTR;
    }

    if( _count == 0 )
    {
        return AE_NULLPTR;
    }

    value = AE_NEWN( _reader->instance, ae_uint8_t, _size * _count );

    if( value == AE_NULLPTR )
    {
        _reader->failed = AE_TRUE;

        return AE_NULLPTR;
    }

    memset( value, 0, _size * _count );

    return value;
}

//////////////////////////////////////////////////////////////////////////
static ae_uint32_t motion_checksum( const ae_uint8_t * _bytes, ae_size_t _size )
{
    ae_uint32_t hash = 2166136261U;
    ae_size_t index;

    for( index = 0; index != _size; ++index )
    {
        hash = ( hash ^ _bytes[index] ) * 16777619U;
    }

    return hash;
}

//////////////////////////////////////////////////////////////////////////
static ae_bool_t motion_validate_program( aeMovieMotionData * _motion, ae_uint32_t _index )
{
    aeMotionTrack * track = _motion->tracks + _index;
    ae_int32_t depth = 0;
    ae_uint32_t index;

    for( index = 0; index != track->instruction_count; ++index )
    {
        const aeMotionInstruction * instruction = track->instructions + index;
        ae_uint32_t opcode = instruction->opcode;
        ae_int32_t arguments = 0;

        if( opcode > AE_MOTION_BASE_REF )
        {
            return AE_FALSE;
        }

        if( opcode == AE_MOTION_PARAM && instruction->index >= _motion->parameter_count )
        {
            return AE_FALSE;
        }

        if( opcode == AE_MOTION_REF && instruction->index >= _index )
        {
            return AE_FALSE;
        }

        if( opcode == AE_MOTION_BASE_REF && instruction->index >= _motion->track_count )
        {
            return AE_FALSE;
        }

        if( opcode == AE_MOTION_REF && track->depth <= _motion->tracks[instruction->index].depth )
        {
            track->depth = _motion->tracks[instruction->index].depth + 1;
        }

        if( track->depth > 64 )
        {
            return AE_FALSE;
        }

        if( opcode >= AE_MOTION_ADD && opcode <= AE_MOTION_MOD )
        {
            arguments = 2;
        }

        if( opcode >= AE_MOTION_NEG && opcode <= AE_MOTION_SQRT )
        {
            arguments = 1;
        }

        if( opcode == AE_MOTION_MIN || opcode == AE_MOTION_MAX || opcode == AE_MOTION_POW )
        {
            arguments = 2;
        }

        if( opcode >= AE_MOTION_CLAMP && opcode <= AE_MOTION_SMOOTH )
        {
            arguments = 3;
        }

        if( opcode == AE_MOTION_NOISE )
        {
            arguments = 2;
        }

        if( opcode == AE_MOTION_WIGGLE || opcode == AE_MOTION_SPRING )
        {
            arguments = 4;
        }

        if( opcode >= AE_MOTION_LT && opcode <= AE_MOTION_NE )
        {
            arguments = 2;
        }

        if( opcode == AE_MOTION_SELECT )
        {
            arguments = 3;
        }

        if( depth < arguments )
        {
            return AE_FALSE;
        }

        depth += 1 - arguments;

        if( depth > (ae_int32_t)MOTION_STACK )
        {
            return AE_FALSE;
        }
    }

    if( track->instruction_count == 0 )
    {
        return AE_TRUE;
    }

    if( depth != 1 )
    {
        return AE_FALSE;
    }

    return AE_TRUE;
}

//////////////////////////////////////////////////////////////////////////
ae_result_t ae_movie_motion_load( aeMovieData * _data, aeMovieStream * _stream )
{
    const aeMovieInstance * instance = _data->instance;
    ae_uint32_t size = 0, checksum = 0, index, subindex;
    ae_uint8_t * payload;
    MotionReader reader;
    aeMovieMotionData * motion;
    ae_result_t result = AE_RESULT_INVALID_DATA;
    AE_READ( _stream, size );
    AE_READ( _stream, checksum );

    if( size < 12 )
    {
        return AE_RESULT_INVALID_DATA;
    }

    if( size > 67108864U )
    {
        return AE_RESULT_INVALID_DATA;
    }

    payload = AE_NEWN( instance, ae_uint8_t, size );

    if( payload == AE_NULLPTR )
    {
        return AE_RESULT_INVALID_MEMORY;
    }

    if( _stream->memory_read( payload, _stream->carriage, size, _stream->read_userdata ) != size )
    {
        AE_DELETEN( instance, payload );

        return AE_RESULT_INVALID_STREAM;
    }

    _stream->carriage += size;

    if( motion_checksum( payload, size ) != checksum )
    {
        AE_DELETEN( instance, payload );

        return AE_RESULT_INVALID_DATA;
    }

    reader.bytes = payload;
    reader.size = size;
    reader.position = 0;
    reader.failed = AE_FALSE;
    reader.instance = instance;
    motion = AE_NEW( instance, aeMovieMotionData );

    if( motion == AE_NULLPTR )
    {
        result = AE_RESULT_INVALID_MEMORY;
        goto finish;
    }

    memset( motion, 0, sizeof( *motion ) );
    _data->motion = motion;
    motion->parameter_count = motion_u32( &reader );
    motion->parameters = (aeMotionParameter *)motion_array( &reader, motion->parameter_count, sizeof( aeMotionParameter ) );

    if( reader.failed == AE_TRUE )
    {
        goto invalid;
    }

    for( index = 0; index != motion->parameter_count; ++index )
    {
        motion->parameters[index].name = motion_string( &reader );
        motion->parameters[index].value = motion_float( &reader );

        if( reader.failed == AE_TRUE )
        {
            goto invalid;
        }

        motion->parameters[index].slot = motion_parameter_slot( motion->parameters[index].name );

        if( motion->parameters[index].slot == AE_MOVIE_MOTION_PARAMETER_COUNT )
        {
            motion->parameters[index].slot += motion->register_count++;
        }
    }

    motion->track_count = motion_u32( &reader );
    motion->tracks = (aeMotionTrack *)motion_array( &reader, motion->track_count, sizeof( aeMotionTrack ) );

    if( reader.failed == AE_TRUE )
    {
        goto invalid;
    }

    for( index = 0; index != motion->track_count; ++index )
    {
        aeMotionTrack * track = motion->tracks + index;
        track->name = motion_string( &reader );
        track->composition = motion_u32( &reader );
        track->layer = motion_u32( &reader );
        track->property = motion_u32( &reader );
        track->extrapolation = motion_u32( &reader );
        track->value = motion_float( &reader );

        if( track->composition >= _data->composition_count )
        {
            goto invalid;
        }

        if( track->layer >= _data->compositions[track->composition].layer_count )
        {
            goto invalid;
        }

        if( track->property > AE_MOTION_CUSTOM )
        {
            goto invalid;
        }

        if( track->extrapolation > 2 )
        {
            goto invalid;
        }

        track->key_count = motion_u32( &reader );
        track->keys = (aeMotionKey *)motion_array( &reader, track->key_count, sizeof( aeMotionKey ) );

        if( reader.failed == AE_TRUE )
        {
            goto invalid;
        }

        for( subindex = 0; subindex != track->key_count; ++subindex )
        {
            aeMotionKey * key = track->keys + subindex;
            key->time = motion_float( &reader );
            key->value = motion_float( &reader );
            key->in_time = motion_float( &reader );
            key->in_value = motion_float( &reader );
            key->out_time = motion_float( &reader );
            key->out_value = motion_float( &reader );
            key->interpolation = motion_u32( &reader );

            if( key->interpolation > 2 )
            {
                goto invalid;
            }

            if( key->time < 0 )
            {
                goto invalid;
            }

            if( subindex > 0 && key->time <= track->keys[subindex - 1].time )
            {
                goto invalid;
            }

            if( subindex > 0 && track->keys[subindex - 1].interpolation == 2 )
            {
                const aeMotionKey * previous = key - 1;

                if( previous->out_time < previous->time )
                {
                    goto invalid;
                }

                if( previous->out_time > key->time )
                {
                    goto invalid;
                }

                if( key->in_time < previous->time )
                {
                    goto invalid;
                }

                if( key->in_time > key->time )
                {
                    goto invalid;
                }
            }
        }

        track->instruction_count = motion_u32( &reader );

        if( track->instruction_count > 4096 )
        {
            goto invalid;
        }

        track->instructions = (aeMotionInstruction *)motion_array( &reader, track->instruction_count, sizeof( aeMotionInstruction ) );

        if( reader.failed == AE_TRUE )
        {
            goto invalid;
        }

        for( subindex = 0; subindex != track->instruction_count; ++subindex )
        {
            track->instructions[subindex].opcode = motion_u32( &reader );
            track->instructions[subindex].index = motion_u32( &reader );
            track->instructions[subindex].value = motion_float( &reader );
        }

        if( motion_validate_program( motion, index ) == AE_FALSE )
        {
            goto invalid;
        }

        for( subindex = 0; subindex != track->instruction_count; ++subindex )
        {
            aeMotionInstruction * instruction = track->instructions + subindex;

            if( instruction->opcode == AE_MOTION_PARAM )
            {
                instruction->index = motion->parameters[instruction->index].slot;
            }
        }
    }

    for( index = 0, subindex = 0; index != motion->parameter_count; ++index )
    {
        aeMotionParameter parameter = motion->parameters[index];

        if( parameter.slot < AE_MOVIE_MOTION_PARAMETER_COUNT )
        {
            continue;
        }

        motion->parameters[index] = motion->parameters[subindex];
        motion->parameters[subindex++] = parameter;
    }

    motion->path_count = motion_u32( &reader );
    motion->paths = (aeMotionPath *)motion_array( &reader, motion->path_count, sizeof( aeMotionPath ) );

    if( reader.failed == AE_TRUE )
    {
        goto invalid;
    }

    for( index = 0; index != motion->path_count; ++index )
    {
        aeMotionPath * path = motion->paths + index;
        path->composition = motion_u32( &reader );
        path->layer = motion_u32( &reader );
        path->channel = motion_u32( &reader );
        path->count = motion_u32( &reader );

        if( path->composition >= _data->composition_count )
        {
            goto invalid;
        }

        if( path->layer >= _data->compositions[path->composition].layer_count )
        {
            goto invalid;
        }

        if( path->channel >= motion->track_count )
        {
            goto invalid;
        }

        if( path->count < 4 )
        {
            goto invalid;
        }

        if( ( path->count - 1 ) % 3 != 0 )
        {
            goto invalid;
        }

        path->points = (ae_vector2_t *)motion_array( &reader, path->count, sizeof( ae_vector2_t ) );

        if( reader.failed == AE_TRUE )
        {
            goto invalid;
        }

        for( subindex = 0; subindex != path->count; ++subindex )
        {
            path->points[subindex][0] = motion_float( &reader );
            path->points[subindex][1] = motion_float( &reader );
        }
    }

    motion->marker_count = motion_u32( &reader );
    motion->markers = (aeMotionMarker *)motion_array( &reader, motion->marker_count, sizeof( aeMotionMarker ) );

    if( reader.failed == AE_TRUE )
    {
        goto invalid;
    }

    for( index = 0; index != motion->marker_count; ++index )
    {
        aeMotionMarker * marker = motion->markers + index;
        marker->name = motion_string( &reader );
        marker->composition = motion_u32( &reader );
        marker->time = motion_float( &reader );

        if( marker->composition >= _data->composition_count )
        {
            goto invalid;
        }

        if( marker->time < 0 )
        {
            goto invalid;
        }

        if( marker->time > _data->compositions[marker->composition].duration_time )
        {
            goto invalid;
        }
    }

    if( reader.failed == AE_TRUE )
    {
        goto invalid;
    }

    if( reader.position != reader.size )
    {
        goto invalid;
    }

    result = AE_RESULT_SUCCESSFUL;
    goto finish;
invalid:
    result = AE_RESULT_INVALID_DATA;
finish:
    AE_DELETEN( instance, payload );

    return result;
}

//////////////////////////////////////////////////////////////////////////
ae_void_t ae_movie_motion_delete( const aeMovieData * _data )
{
    const aeMovieInstance * instance = _data->instance;
    aeMovieMotionData * motion = _data->motion;
    ae_uint32_t index;

    if( motion == AE_NULLPTR )
    {
        return;
    }

    if( motion->parameters != AE_NULLPTR )
    {
        for( index = 0; index != motion->parameter_count; ++index )
        {
            AE_DELETEN( instance, motion->parameters[index].name );
        }
    }

    if( motion->tracks != AE_NULLPTR )
    {
        for( index = 0; index != motion->track_count; ++index )
        {
            AE_DELETEN( instance, motion->tracks[index].name );
            AE_DELETEN( instance, motion->tracks[index].keys );
            AE_DELETEN( instance, motion->tracks[index].instructions );
        }
    }

    if( motion->markers != AE_NULLPTR )
    {
        for( index = 0; index != motion->marker_count; ++index )
        {
            AE_DELETEN( instance, motion->markers[index].name );
        }
    }

    if( motion->paths != AE_NULLPTR )
    {
        for( index = 0; index != motion->path_count; ++index )
        {
            AE_DELETEN( instance, motion->paths[index].points );
        }
    }

    AE_DELETEN( instance, motion->parameters );
    AE_DELETEN( instance, motion->tracks );
    AE_DELETEN( instance, motion->paths );
    AE_DELETEN( instance, motion->markers );
    AE_DELETE( instance, motion );
}

//////////////////////////////////////////////////////////////////////////
ae_bool_t ae_movie_motion_create_state( aeMovieComposition * _composition )
{
    const aeMovieInstance * instance = _composition->instance;
    const aeMovieMotionData * motion = _composition->composition_data->movie_data->motion;
    aeMovieMotionState * state;
    ae_uint32_t index;

    if( motion == AE_NULLPTR )
    {
        return AE_TRUE;
    }

    state = AE_NEW( instance, aeMovieMotionState );

    if( state == AE_NULLPTR )
    {
        return AE_FALSE;
    }

    memset( state, 0, sizeof( *state ) );
    _composition->motion = state;
    state->parameters = AE_NEWN( instance, ae_float_t, AE_MOVIE_MOTION_PARAMETER_COUNT + motion->register_count );
    state->values = motion->track_count ? AE_NEWN( instance, ae_float_t, motion->track_count ) : AE_NULLPTR;
    state->times = motion->track_count ? AE_NEWN( instance, ae_float_t, motion->track_count ) : AE_NULLPTR;
    state->revisions = motion->track_count ? AE_NEWN( instance, uint64_t, motion->track_count ) : AE_NULLPTR;
    state->parameter_revision = 1;
    state->pose_revision = 1;

    if( state->parameters == AE_NULLPTR )
    {
        ae_movie_motion_delete_state( _composition );
        _composition->motion = AE_NULLPTR;

        return AE_FALSE;
    }

    if( ( motion->track_count != 0 && ( state->values == AE_NULLPTR || state->times == AE_NULLPTR || state->revisions == AE_NULLPTR ) ) )
    {
        ae_movie_motion_delete_state( _composition );
        _composition->motion = AE_NULLPTR;

        return AE_FALSE;
    }

    for( index = 0; index != AE_MOVIE_MOTION_PARAMETER_COUNT; ++index )
    {
        state->parameters[index] = MOTION_PARAMETER_DEFAULTS[index].value;
    }

    for( index = 0; index != motion->parameter_count; ++index )
    {
        state->parameters[motion->parameters[index].slot] = motion->parameters[index].value;
    }

    if( motion->track_count != 0 )
    {
        memset( state->revisions, 0, motion->track_count * sizeof( uint64_t ) );
    }

    for( index = 0; index != _composition->node_count; ++index )
    {
        _composition->nodes[index].immutable_matrix = AE_FALSE;
        _composition->nodes[index].immutable_color = AE_FALSE;
    }

    return AE_TRUE;
}

//////////////////////////////////////////////////////////////////////////
ae_void_t ae_movie_motion_delete_state( const aeMovieComposition * _composition )
{
    const aeMovieInstance * instance = _composition->instance;
    aeMovieMotionState * state = _composition->motion;

    if( state == AE_NULLPTR )
    {
        return;
    }

    AE_DELETEN( instance, state->parameters );
    AE_DELETEN( instance, state->values );
    AE_DELETEN( instance, state->times );
    AE_DELETEN( instance, state->revisions );
    AE_DELETE( instance, state );
}

//////////////////////////////////////////////////////////////////////////
static ae_float_t motion_bezier( ae_float_t _a, ae_float_t _b, ae_float_t _c, ae_float_t _d, ae_float_t _t )
{
    ae_float_t u = 1.f - _t;

    ae_float_t value = u * u * u * _a + 3.f * u * u * _t * _b + 3.f * u * _t * _t * _c + _t * _t * _t * _d;

    return value;
}

//////////////////////////////////////////////////////////////////////////
static ae_float_t motion_base( const aeMotionTrack * _track, ae_float_t _time )
{
    const aeMotionKey * first;
    const aeMotionKey * last;
    ae_uint32_t low, high;
    ae_float_t progress;

    if( _track->key_count == 0 )
    {
        return _track->value;
    }

    first = _track->keys;
    last = first + _track->key_count - 1;

    if( _track->extrapolation != 0 && last->time > first->time && ( _time > last->time || _time < first->time ) )
    {
        ae_float_t length = last->time - first->time;
        ae_float_t repeat = ( _time - first->time ) / length;
        ae_float_t cycle = floorf( repeat );
        progress = repeat - cycle;

        if( _track->extrapolation == 2 && fmodf( fabsf( cycle ), 2.f ) >= 1.f )
        {
            progress = 1.f - progress;
        }

        _time = first->time + progress * length;
    }

    if( _time <= first->time )
    {
        return first->value;
    }

    if( _time >= last->time )
    {
        return last->value;
    }

    low = 0;
    high = _track->key_count - 1;

    while( high - low > 1 )
    {
        ae_uint32_t middle = ( low + high ) / 2;

        if( _track->keys[middle].time <= _time )
        {
            low = middle;
        }
        else
        {
            high = middle;
        }
    }

    first = _track->keys + low;
    last = first + 1;

    if( first->interpolation == 0 )
    {
        return first->value;
    }

    progress = ( _time - first->time ) / ( last->time - first->time );

    if( first->interpolation == 2 )
    {
        ae_float_t left = 0.f, right = 1.f;
        ae_uint32_t iteration;

        for( iteration = 0; iteration != 26; ++iteration )
        {
            progress = ( left + right ) * .5f;

            if( motion_bezier( first->time, first->out_time, last->in_time, last->time, progress ) < _time )
            {
                left = progress;
            }
            else
            {
                right = progress;
            }
        }

        ae_float_t value = motion_bezier( first->value, first->out_value, last->in_value, last->value, progress );

        return value;
    }

    ae_float_t value = first->value + ( last->value - first->value ) * progress;

    return value;
}

//////////////////////////////////////////////////////////////////////////
ae_float_t ae_movie_motion_sample_keys( const aeMotionKey * _keys, ae_uint32_t _count, ae_float_t _time )
{
    aeMotionTrack track;
    memset( &track, 0, sizeof( track ) );
    track.keys = (aeMotionKey *)_keys;
    track.key_count = _count;
    ae_float_t value = motion_base( &track, _time );

    return value;
}

//////////////////////////////////////////////////////////////////////////
static ae_float_t motion_noise( ae_float_t _time, ae_float_t _seed )
{
    ae_float_t floor_time;

    if( isfinite( _time ) == 0 )
    {
        return 0.f;
    }

    if( isfinite( _seed ) == 0 )
    {
        return 0.f;
    }

    floor_time = floorf( _time );
    uint32_t cell = (uint32_t)(int64_t)fmod( (double)floor_time, 2147483647.0 );
    uint32_t seed = (uint32_t)(int64_t)fmod( (double)_seed, 2147483647.0 );
    ae_float_t values[2], fraction = _time - floor_time;
    ae_uint32_t index;

    for( index = 0; index != 2; ++index )
    {
        uint32_t hash = ( cell + index ) ^ ( seed * 0x9e3779b9U );
        hash ^= hash >> 16;
        hash *= 0x7feb352dU;
        hash ^= hash >> 15;
        hash *= 0x846ca68bU;
        hash ^= hash >> 16;
        values[index] = (ae_float_t)( hash & 0x00ffffffU ) / 8388607.5f - 1.f;
    }

    fraction = fraction * fraction * ( 3.f - 2.f * fraction );

    ae_float_t value = values[0] + ( values[1] - values[0] ) * fraction;

    return value;
}

//////////////////////////////////////////////////////////////////////////
ae_float_t ae_movie_motion_channel_value( const aeMovieComposition * _composition, ae_uint32_t _index, ae_float_t _time )
{
    const aeMovieMotionData * motion = _composition->composition_data->movie_data->motion;
    aeMovieMotionState * state = _composition->motion;
    const aeMotionTrack * track = motion->tracks + _index;
    ae_float_t base, stack[MOTION_STACK], value;
    ae_uint32_t count = 0, index;

    if( state->revisions[_index] == state->parameter_revision && state->times[_index] == _time )
    {
        return state->values[_index];
    }

    base = motion_base( track, _time );
    value = base;

    for( index = 0; index != track->instruction_count; ++index )
    {
        const aeMotionInstruction * instruction = track->instructions + index;
        ae_uint32_t opcode = instruction->opcode;
        ae_float_t a, b, c, d;

        switch( opcode )
        {
        case AE_MOTION_CONST:
            {
                stack[count++] = instruction->value;
            }break;
        case AE_MOTION_TIME:
            {
                stack[count++] = _time;
            }break;
        case AE_MOTION_VALUE:
            {
                stack[count++] = base;
            }break;
        case AE_MOTION_PARAM:
            {
                stack[count++] = state->parameters[instruction->index];
            }break;
        case AE_MOTION_REF:
            {
                stack[count++] = ae_movie_motion_channel_value( _composition, instruction->index, _time );
            }break;
        case AE_MOTION_BASE_REF:
            {
                stack[count++] = motion_base( motion->tracks + instruction->index, _time );
            }break;
        case AE_MOTION_NEG:
            {
                stack[count - 1] = -stack[count - 1];
            }break;
        case AE_MOTION_SIN:
            {
                stack[count - 1] = sinf( stack[count - 1] );
            }break;
        case AE_MOTION_COS:
            {
                stack[count - 1] = cosf( stack[count - 1] );
            }break;
        case AE_MOTION_ABS:
            {
                stack[count - 1] = fabsf( stack[count - 1] );
            }break;
        case AE_MOTION_EXP:
            {
                stack[count - 1] = expf( stack[count - 1] );
            }break;
        case AE_MOTION_SQRT:
            {
                stack[count - 1] = sqrtf( stack[count - 1] );
            }break;
        default:
            {
                b = stack[--count];
                a = stack[--count];
                c = 0.f;

                switch( opcode )
                {
                case AE_MOTION_ADD:
                    {
                        c = a + b;
                    }break;
                case AE_MOTION_SUB:
                    {
                        c = a - b;
                    }break;
                case AE_MOTION_MUL:
                    {
                        c = a * b;
                    }break;
                case AE_MOTION_DIV:
                    {
                        c = a / b;
                    }break;
                case AE_MOTION_MOD:
                    {
                        c = fmodf( a, b );
                    }break;
                case AE_MOTION_MIN:
                    {
                        c = fminf( a, b );
                    }break;
                case AE_MOTION_MAX:
                    {
                        c = fmaxf( a, b );
                    }break;
                case AE_MOTION_POW:
                    {
                        c = powf( a, b );
                    }break;
                case AE_MOTION_LT:
                    {
                        c = a < b;
                    }break;
                case AE_MOTION_LE:
                    {
                        c = a <= b;
                    }break;
                case AE_MOTION_GT:
                    {
                        c = a > b;
                    }break;
                case AE_MOTION_GE:
                    {
                        c = a >= b;
                    }break;
                case AE_MOTION_EQ:
                    {
                        c = a == b;
                    }break;
                case AE_MOTION_NE:
                    {
                        c = a != b;
                    }break;
                case AE_MOTION_NOISE:
                    {
                        c = motion_noise( a, b );
                    }break;
                case AE_MOTION_CLAMP:
                    {
                        d = stack[--count];
                        c = fminf( fmaxf( d, a ), b );
                    }break;
                case AE_MOTION_LERP:
                    {
                        d = stack[--count];
                        c = d + ( a - d ) * b;
                    }break;
                case AE_MOTION_SMOOTH:
                    {
                        d = stack[--count];
                        c = a == d ? 0.f : fminf( fmaxf( ( b - d ) / ( a - d ), 0.f ), 1.f );
                        c = c * c * ( 3.f - 2.f * c );
                    }break;
                case AE_MOTION_SELECT:
                    {
                        d = stack[--count];
                        c = d != 0.f ? a : b;
                    }break;
                case AE_MOTION_WIGGLE:
                    {
                        c = stack[--count];
                        d = stack[--count];
                        c = c * motion_noise( a * d, b );
                    }break;
                case AE_MOTION_SPRING:
                    {
                        c = stack[--count];
                        d = stack[--count];
                        c = d * expf( -a * b ) * sinf( 6.28318530718f * c * b );
                    }break;
                default:
                    {
                    }break;
                }

                stack[count++] = c;
            }break;
        }
    }

    if( count == 1 )
    {
        value = stack[0];
    }

    if( isfinite( value ) == 0 )
    {
        value = base;
    }

    state->revisions[_index] = state->parameter_revision;
    state->times[_index] = _time;
    state->values[_index] = value;

    return value;
}

//////////////////////////////////////////////////////////////////////////
ae_void_t ae_movie_motion_update_node( aeMovieNode * _node, const aeMovieComposition * _composition, ae_uint32_t _frame, ae_float_t _fraction )
{
    const aeMovieLayerData * layer = _node->layer_data;
    const aeMovieMotionData * motion = layer->composition_data->movie_data->motion;
    ae_uint32_t composition_index = (ae_uint32_t)( layer->composition_data - layer->composition_data->movie_data->compositions );
    ae_uint32_t layer_index = (ae_uint32_t)( layer - layer->composition_data->layers );
    ae_float_t time = ( (ae_float_t)_frame + _fraction ) * layer->composition_data->frame_duration;
    ae_float_t properties[AE_MOTION_CUSTOM] = { 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0 };
    ae_vector2_t position, anchor, scale;
    ae_quaternionzw_t quaternion;
    ae_skew_t skew = { 0, 0, 1 };
    ae_matrix34_t local;
    ae_uint32_t index;
    aeMovieNode * parent = _node->relative_node;

    for( index = 0; index != motion->track_count; ++index )
    {
        const aeMotionTrack * track = motion->tracks + index;

        if( track->composition == composition_index && track->layer == layer_index && track->property < AE_MOTION_CUSTOM )
        {
            properties[track->property] = ae_movie_motion_channel_value( _composition, index, time );
        }
    }

    position[0] = properties[AE_MOTION_X];
    position[1] = properties[AE_MOTION_Y];

    for( index = 0; index != motion->path_count; ++index )
    {
        const aeMotionPath * path = motion->paths + index;

        if( path->composition == composition_index && path->layer == layer_index )
        {
            ae_float_t progress = fminf( fmaxf( ae_movie_motion_channel_value( _composition, path->channel, time ), 0.f ), 1.f );
            ae_uint32_t segments = ( path->count - 1 ) / 3;
            ae_float_t segment_time = progress * (ae_float_t)segments;
            ae_uint32_t segment = (ae_uint32_t)segment_time;
            const ae_vector2_t * points;

            if( segment == segments )
            {
                segment = segments - 1;
                segment_time = (ae_float_t)segments;
            }

            points = path->points + segment * 3;
            position[0] += motion_bezier( points[0][0], points[1][0], points[2][0], points[3][0], segment_time - (ae_float_t)segment );
            position[1] += motion_bezier( points[0][1], points[1][1], points[2][1], points[3][1], segment_time - (ae_float_t)segment );
        }
    }

    anchor[0] = properties[AE_MOTION_ANCHOR_X];
    anchor[1] = properties[AE_MOTION_ANCHOR_Y];
    scale[0] = properties[AE_MOTION_SCALE_X];
    scale[1] = properties[AE_MOTION_SCALE_Y];
    quaternion[0] = sinf( properties[AE_MOTION_ROTATION] * .00872664625997f );
    quaternion[1] = cosf( properties[AE_MOTION_ROTATION] * .00872664625997f );
    skew[0] = properties[AE_MOTION_SKEW] * .01745329251994f;
    ae_movie_make_transformation2d_m34( local, position, anchor, scale, quaternion, skew );

    if( _node->extra_transformation != AE_NULLPTR )
    {
        _node->extra_transformation( local, _frame, _fraction, _node->extra_transformation_userdata );
    }

    if( parent != AE_NULLPTR )
    {
        ae_mul_m34_m34_r( _node->matrix, local, parent->matrix );
    }
    else
    {
        ae_copy_m34( _node->matrix, local );
    }

    _node->volume = properties[AE_MOTION_VOLUME];
    _node->color.r = properties[AE_MOTION_RED] * ( parent ? parent->composition_color.r : 1.f );
    _node->color.g = properties[AE_MOTION_GREEN] * ( parent ? parent->composition_color.g : 1.f );
    _node->color.b = properties[AE_MOTION_BLUE] * ( parent ? parent->composition_color.b : 1.f );
    _node->opacity = properties[AE_MOTION_OPACITY] * ( parent ? parent->composition_opacity : 1.f );

    if( layer->subcomposition_data != AE_NULLPTR )
    {
        _node->composition_color = _node->color;
        _node->composition_opacity = _node->opacity;
    }
    else
    {
        _node->composition_color.r = parent ? parent->composition_color.r : 1.f;
        _node->composition_color.g = parent ? parent->composition_color.g : 1.f;
        _node->composition_color.b = parent ? parent->composition_color.b : 1.f;
        _node->composition_opacity = parent ? parent->composition_opacity : 1.f;
    }
}

//////////////////////////////////////////////////////////////////////////
ae_bool_t ae_is_movie_motion_data( const aeMovieData * _data )
{
    if( _data->motion == AE_NULLPTR )
    {
        return AE_FALSE;
    }

    return AE_TRUE;
}

//////////////////////////////////////////////////////////////////////////
ae_uint32_t ae_get_movie_motion_channel_count( const aeMovieData * _data )
{
    if( _data->motion == AE_NULLPTR )
    {
        return 0;
    }

    return _data->motion->track_count;
}

const ae_char_t * ae_get_movie_motion_channel_name( const aeMovieData * _data, ae_uint32_t _index )
{
    if( _data->motion == AE_NULLPTR )
    {
        return AE_NULLPTR;
    }

    if( _index >= _data->motion->track_count )
    {
        return AE_NULLPTR;
    }

    return _data->motion->tracks[_index].name;
}

//////////////////////////////////////////////////////////////////////////
ae_bool_t ae_evaluate_movie_motion_channel( const aeMovieComposition * _composition, const ae_char_t * _name, ae_time_t _time, ae_float_t * _value )
{
    const aeMovieMotionData * motion = _composition->composition_data->movie_data->motion;
    ae_uint32_t index;

    if( motion == AE_NULLPTR )
    {
        return AE_FALSE;
    }

    if( isfinite( _time ) == 0 )
    {
        return AE_FALSE;
    }

    if( _value == AE_NULLPTR )
    {
        return AE_FALSE;
    }

    if( _name == AE_NULLPTR )
    {
        return AE_FALSE;
    }

    for( index = 0; index != motion->track_count; ++index )
    {
        if( strcmp( motion->tracks[index].name, _name ) != 0 )
        {
            continue;
        }

        *_value = ae_movie_motion_channel_value( _composition, index, _time );

        return AE_TRUE;
    }

    return AE_FALSE;
}

//////////////////////////////////////////////////////////////////////////
static ae_bool_t motion_set_parameter( const aeMovieComposition * _composition, ae_uint32_t _slot, ae_float_t _value )
{
    aeMovieMotionState * state = _composition->motion;

    if( state == AE_NULLPTR )
    {
        return AE_FALSE;
    }

    if( isfinite( _value ) == 0 )
    {
        return AE_FALSE;
    }

    if( state->parameters[_slot] == _value )
    {
        return AE_TRUE;
    }

    state->parameters[_slot] = _value;
    ++state->parameter_revision;

    return AE_TRUE;
}

//////////////////////////////////////////////////////////////////////////
ae_bool_t ae_set_movie_motion_parameter( const aeMovieComposition * _composition, aeMovieMotionParameter _parameter, ae_float_t _value )
{
    if( (ae_uint32_t)_parameter >= AE_MOVIE_MOTION_PARAMETER_COUNT )
    {
        return AE_FALSE;
    }

    if( motion_set_parameter( _composition, (ae_uint32_t)_parameter, _value ) == AE_FALSE )
    {
        return AE_FALSE;
    }

    return AE_TRUE;
}

//////////////////////////////////////////////////////////////////////////
ae_bool_t ae_get_movie_motion_parameter( const aeMovieComposition * _composition, aeMovieMotionParameter _parameter, ae_float_t * _value )
{
    if( _composition->motion == AE_NULLPTR )
    {
        return AE_FALSE;
    }

    if( (ae_uint32_t)_parameter >= AE_MOVIE_MOTION_PARAMETER_COUNT )
    {
        return AE_FALSE;
    }

    if( _value == AE_NULLPTR )
    {
        return AE_FALSE;
    }

    *_value = _composition->motion->parameters[_parameter];

    return AE_TRUE;
}

//////////////////////////////////////////////////////////////////////////
ae_uint32_t ae_get_movie_motion_register_count( const aeMovieData * _data )
{
    if( _data->motion == AE_NULLPTR )
    {
        return 0;
    }

    return _data->motion->register_count;
}

//////////////////////////////////////////////////////////////////////////
const ae_char_t * ae_get_movie_motion_register_name( const aeMovieData * _data, ae_uint32_t _index )
{
    ae_uint32_t count = ae_get_movie_motion_register_count( _data );

    if( _index >= count )
    {
        return AE_NULLPTR;
    }

    return _data->motion->parameters[_index].name;
}

//////////////////////////////////////////////////////////////////////////
ae_bool_t ae_find_movie_motion_register( const aeMovieData * _data, const ae_char_t * _name, ae_uint32_t * _index )
{
    ae_uint32_t parameter;

    if( _data->motion == AE_NULLPTR )
    {
        return AE_FALSE;
    }

    if( _name == AE_NULLPTR )
    {
        return AE_FALSE;
    }

    if( _index == AE_NULLPTR )
    {
        return AE_FALSE;
    }

    for( parameter = 0; parameter != _data->motion->register_count; ++parameter )
    {
        const aeMotionParameter * value = _data->motion->parameters + parameter;

        if( strcmp( value->name, _name ) != 0 )
        {
            continue;
        }

        *_index = parameter;

        return AE_TRUE;
    }

    return AE_FALSE;
}

//////////////////////////////////////////////////////////////////////////
ae_bool_t ae_set_movie_motion_register( const aeMovieComposition * _composition, ae_uint32_t _index, ae_float_t _value )
{
    const aeMovieMotionData * motion = _composition->composition_data->movie_data->motion;

    if( motion == AE_NULLPTR )
    {
        return AE_FALSE;
    }

    if( _index >= motion->register_count )
    {
        return AE_FALSE;
    }

    ae_uint32_t slot = AE_MOVIE_MOTION_PARAMETER_COUNT + _index;

    if( motion_set_parameter( _composition, slot, _value ) == AE_FALSE )
    {
        return AE_FALSE;
    }

    return AE_TRUE;
}

//////////////////////////////////////////////////////////////////////////
ae_bool_t ae_get_movie_motion_register( const aeMovieComposition * _composition, ae_uint32_t _index, ae_float_t * _value )
{
    const aeMovieMotionData * motion = _composition->composition_data->movie_data->motion;

    if( motion == AE_NULLPTR )
    {
        return AE_FALSE;
    }

    if( _index >= motion->register_count )
    {
        return AE_FALSE;
    }

    if( _value == AE_NULLPTR )
    {
        return AE_FALSE;
    }

    *_value = _composition->motion->parameters[AE_MOVIE_MOTION_PARAMETER_COUNT + _index];

    return AE_TRUE;
}

//////////////////////////////////////////////////////////////////////////
ae_void_t ae_set_movie_marker_callback( const aeMovieComposition * _composition, ae_movie_marker_callback_t _callback, ae_userdata_t _userdata )
{
    if( _composition->motion == AE_NULLPTR )
    {
        return;
    }

    _composition->motion->callback = _callback;
    _composition->motion->userdata = _userdata;
}

//////////////////////////////////////////////////////////////////////////
static ae_void_t motion_markers( const aeMovieComposition * _composition, ae_float_t _begin, ae_float_t _end, ae_bool_t _include_begin )
{
    const aeMovieMotionData * motion = _composition->composition_data->movie_data->motion;
    aeMovieMotionState * state = _composition->motion;
    ae_uint32_t index;
    ae_uint32_t composition_index = (ae_uint32_t)( _composition->composition_data - _composition->composition_data->movie_data->compositions );

    if( state->callback == AE_NULLPTR )
    {
        return;
    }

    for( index = 0; index != motion->marker_count; ++index )
    {
        const aeMotionMarker * marker = motion->markers + index;

        if( marker->composition != composition_index )
        {
            continue;
        }

        if( marker->time <= _end && ( marker->time > _begin || ( _include_begin && marker->time == _begin ) ) )
        {
            aeMovieMarkerCallbackData callback_data;
            callback_data.name = marker->name;
            callback_data.time = marker->time;
            callback_data.iteration = state->iteration;
            ( *state->callback )( &callback_data, state->userdata );
        }
    }
}

//////////////////////////////////////////////////////////////////////////
ae_void_t ae_movie_motion_restart( const aeMovieComposition * _composition )
{
    if( _composition->motion != AE_NULLPTR )
    {
        _composition->motion->iteration = 0;
        _composition->motion->started = AE_FALSE;
        _composition->motion->finished = AE_FALSE;
    }
}

//////////////////////////////////////////////////////////////////////////
ae_void_t ae_movie_motion_dispatch( const aeMovieComposition * _composition, ae_time_t _delta )
{
    aeMovieMotionState * state = _composition->motion;
    aeMovieCompositionAnimation * animation = _composition->animation;
    ae_float_t begin, end, limit;
    ae_bool_t include;

    if( state == AE_NULLPTR || animation->play == AE_FALSE || animation->pause == AE_TRUE || _delta <= 0 || isfinite( _delta ) == 0 )
    {
        return;
    }

    begin = animation->time;
    end = begin + _delta;
    limit = animation->work_area_time_end;
    include = !state->started;
    state->started = AE_TRUE;

    if( animation->loop == AE_TRUE && animation->interrupt == AE_FALSE )
    {
        ae_float_t loop_begin = animation->loop_segment_time_begin;
        limit = animation->loop_segment_time_end;

        if( limit > loop_begin )
        {
            while( end >= limit )
            {
                motion_markers( _composition, begin, limit, include );
                end = loop_begin + end - limit;
                begin = loop_begin;
                include = AE_TRUE;
                ++state->iteration;
            }
        }
    }

    motion_markers( _composition, begin, fminf( end, limit ), include );

    if( animation->loop == AE_FALSE && end >= limit )
    {
        state->finished = AE_TRUE;
    }
}

//////////////////////////////////////////////////////////////////////////
ae_bool_t ae_finish_movie_motion( const aeMovieComposition * _composition )
{
    ae_float_t duration;

    if( _composition->motion == AE_NULLPTR )
    {
        return AE_FALSE;
    }

    if( _composition->motion->finished == AE_TRUE )
    {
        return AE_TRUE;
    }

    _composition->motion->finished = AE_TRUE;
    duration = _composition->composition_data->duration_time;
    motion_markers( _composition, _composition->animation->time, duration, !_composition->motion->started );
    _composition->motion->started = AE_TRUE;
    ae_set_movie_composition_time( _composition, duration );
    ae_stop_movie_composition( _composition );

    return AE_TRUE;
}
