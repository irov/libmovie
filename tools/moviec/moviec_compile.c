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

#include "moviec_internal.h"

#include <stdlib.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
typedef struct MoviecMarker
{
    const char * name;
    uint32_t composition;
    float time;
    size_t order;
} MoviecMarker;
//////////////////////////////////////////////////////////////////////////
static int moviec_compare_markers( const void * _left, const void * _right )
{
    const MoviecMarker * left = _left;
    const MoviecMarker * right = _right;

    if( left->composition != right->composition )
    {
        int result = left->composition < right->composition ? -1 : 1;

        return result;
    }

    if( left->time != right->time )
    {
        int result = left->time < right->time ? -1 : 1;

        return result;
    }

    int result = left->order < right->order ? -1 : left->order > right->order ? 1
                                                                              : 0;

    return result;
}
//////////////////////////////////////////////////////////////////////////
static void moviec_compile_paths( MoviecContext * _context, MoviecWriter * _writer )
{
    MoviecWriter paths = { 0 };
    uint32_t count = 0;
    json_t * list = moviec_get( _context->document, "compositions" );

    for( uint32_t compositionIndex = 0; compositionIndex != json_array_size( list ); ++compositionIndex )
    {
        json_t * composition = json_array_get( list, compositionIndex );
        json_t * layers = moviec_get( composition, "layers" );
        const char * id = moviec_text( moviec_get( composition, "id" ), "" );

        for( uint32_t layerIndex = 0; layerIndex != json_array_size( layers ); ++layerIndex )
        {
            json_t * layer = json_array_get( layers, layerIndex );
            json_t * path = moviec_get( layer, "path" );

            if( path == NULL )
            {
                continue;
            }

            _context->path = moviec_format( _context, "/compositions/%u/layers/%u/path", compositionIndex, layerIndex );
            moviec_members( _context, path, "points" );
            json_t * points = moviec_get( path, "points" );
            size_t pointCount = json_array_size( points );
            moviec_require( _context, pointCount >= 4 && ( pointCount - 1 ) % 3 == 0, "path needs 3n+1 cubic control points" );
            const char * name = moviec_format( _context, "%s.%s.pathProgress", id, moviec_text( moviec_get( layer, "id" ), "" ) );
            json_t * indices = moviec_get( _context->channels, name );
            moviec_require( _context, indices != NULL, "path needs a pathProgress channel" );
            moviec_write_u32( _context, &paths, compositionIndex );
            moviec_write_u32( _context, &paths, layerIndex );
            moviec_write_u32( _context, &paths, (uint32_t)json_integer_value( json_array_get( indices, 0 ) ) );
            moviec_write_u32( _context, &paths, (uint32_t)pointCount );

            for( size_t index = 0; index != pointCount; ++index )
            {
                float point[2];
                moviec_vector( _context, json_array_get( points, index ), 2, 0.f, point );
                moviec_write_float( _context, &paths, point[0] );
                moviec_write_float( _context, &paths, point[1] );
            }

            ++count;
        }
    }

    moviec_write_u32( _context, _writer, count );
    moviec_write_raw( _context, _writer, paths.bytes, paths.size );
}
//////////////////////////////////////////////////////////////////////////
static void moviec_compile_markers( MoviecContext * _context, MoviecWriter * _writer )
{
    json_t * list = moviec_get( _context->document, "compositions" );
    size_t count = 0;

    for( size_t index = 0; index != json_array_size( list ); ++index )
    {
        count += json_array_size( moviec_get( json_array_get( list, index ), "markers" ) );
    }

    MoviecMarker * markers = moviec_allocate( _context, count * sizeof( MoviecMarker ) );
    size_t used = 0;

    for( uint32_t compositionIndex = 0; compositionIndex != json_array_size( list ); ++compositionIndex )
    {
        json_t * composition = json_array_get( list, compositionIndex );
        json_t * events = moviec_get( composition, "markers" );
        json_t * ids = moviec_object( _context );
        float duration = moviec_number( _context, moviec_get( composition, "duration" ), 0.f );

        for( size_t index = 0; index != json_array_size( events ); ++index )
        {
            _context->path = moviec_format( _context, "/compositions/%u/markers/%zu", compositionIndex, index );
            json_t * event = json_array_get( events, index );
            moviec_members( _context, event, "id time" );
            moviec_needs( _context, event, "id time" );
            const char * name = moviec_text( moviec_get( event, "id" ), "" );
            moviec_require( _context, name[0] != '\0' && moviec_get( ids, name ) == NULL, "missing or duplicate marker id" );
            moviec_json_integer( _context, ids, name, 1 );
            float time = moviec_number( _context, moviec_get( event, "time" ), 0.f );
            moviec_require( _context, time >= 0.f && time <= duration, "marker outside composition" );
            MoviecMarker marker = { name, compositionIndex, time, used };
            markers[used++] = marker;
        }
    }

    qsort( markers, count, sizeof( MoviecMarker ), moviec_compare_markers );
    moviec_write_u32( _context, _writer, (uint32_t)count );

    for( size_t index = 0; index != count; ++index )
    {
        moviec_write_string( _context, _writer, markers[index].name );
        moviec_write_u32( _context, _writer, markers[index].composition );
        moviec_write_float( _context, _writer, markers[index].time );
    }
}
//////////////////////////////////////////////////////////////////////////
MoviecWriter moviec_compile( MoviecContext * _context )
{
    moviec_collect( _context );
    MoviecWriter scaffold = moviec_scaffold( _context );
    MoviecWriter output = { 0 };
    uint32_t contentChecksum = 2166136261U;

    for( size_t index = 16; index != scaffold.size; ++index )
    {
        contentChecksum = ( contentChecksum ^ scaffold.bytes[index] ) * 16777619U;
    }

    moviec_write_raw( _context, &output, scaffold.bytes, 16 );
    moviec_write_u32( _context, &output, (uint32_t)( scaffold.size - 16 ) );
    moviec_write_u32( _context, &output, contentChecksum );
    moviec_write_raw( _context, &output, scaffold.bytes + 16, scaffold.size - 16 );
    MoviecWriter payload = { 0 };
    moviec_write_u32( _context, &payload, (uint32_t)json_array_size( _context->parameterNames ) );

    for( size_t index = 0; index != json_array_size( _context->parameterNames ); ++index )
    {
        const char * name = json_string_value( json_array_get( _context->parameterNames, index ) );
        moviec_write_string( _context, &payload, name );
        float value = moviec_number( _context, moviec_get( moviec_get( _context->document, "parameters" ), name ), 0.f );
        moviec_write_float( _context, &payload, value );
    }

    moviec_write_u32( _context, &payload, (uint32_t)_context->trackCount );

    for( size_t index = 0; index != _context->trackCount; ++index )
    {
        const MoviecTrack * track = _context->tracks + index;
        moviec_write_string( _context, &payload, track->name );
        moviec_write_u32( _context, &payload, track->composition );
        moviec_write_u32( _context, &payload, track->layer );
        moviec_write_u32( _context, &payload, track->property );
        moviec_write_u32( _context, &payload, track->extrapolation );
        moviec_write_float( _context, &payload, track->value );
        moviec_write_u32( _context, &payload, (uint32_t)track->keyCount );

        for( size_t keyIndex = 0; keyIndex != track->keyCount; ++keyIndex )
        {
            const aeMotionKey * key = track->keys + keyIndex;
            float values[] = { key->time, key->value, key->in_time, key->in_value, key->out_time, key->out_value };

            for( size_t component = 0; component != 6; ++component )
            {
                moviec_write_float( _context, &payload, values[component] );
            }

            moviec_write_u32( _context, &payload, key->interpolation );
        }

        moviec_write_u32( _context, &payload, (uint32_t)track->code.count );

        for( size_t instructionIndex = 0; instructionIndex != track->code.count; ++instructionIndex )
        {
            const aeMotionInstruction * instruction = track->code.instructions + instructionIndex;
            moviec_write_u32( _context, &payload, instruction->opcode );
            moviec_write_u32( _context, &payload, instruction->index );
            moviec_write_float( _context, &payload, instruction->value );
        }
    }

    moviec_compile_paths( _context, &payload );
    moviec_compile_markers( _context, &payload );
    uint32_t checksum = 2166136261U;

    for( size_t index = 0; index != payload.size; ++index )
    {
        checksum = ( checksum ^ payload.bytes[index] ) * 16777619U;
    }

    moviec_write_byte( _context, &output, AE_MOVIE_DATA_EXTENSION_MOTION );
    moviec_write_u32( _context, &output, (uint32_t)payload.size );
    moviec_write_u32( _context, &output, checksum );
    moviec_write_raw( _context, &output, payload.bytes, payload.size );
    moviec_write_byte( _context, &output, 0 );

    return output;
}
