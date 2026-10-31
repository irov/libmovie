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

#include <math.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
static void moviec_release_geometry_points( MoviecContext * _context, size_t _first )
{
    while( json_array_size( _context->ownedJson ) > _first )
    {
        json_array_remove( _context->ownedJson, json_array_size( _context->ownedJson ) - 1 );
    }
}
//////////////////////////////////////////////////////////////////////////
static void moviec_write_point( MoviecContext * _context, MoviecWriter * _writer, json_t * _point )
{
    float values[2];
    moviec_vector( _context, _point, 2, 0.f, values );
    moviec_write_float( _context, _writer, values[0] );
    moviec_write_float( _context, _writer, values[1] );
}
//////////////////////////////////////////////////////////////////////////
static void moviec_mesh( MoviecContext * _context, MoviecWriter * _writer, json_t * _vertices, json_t * _uvs, json_t * _indices )
{
    size_t count = json_array_size( _vertices );
    moviec_require( _context, count >= 3 && count <= AE_MOVIE_MAX_VERTICES && json_array_size( _uvs ) == count, "mesh needs matching vertices and UVs" );
    moviec_require( _context, json_array_size( _indices ) > 0 && json_array_size( _indices ) % 3 == 0, "mesh needs triangle indices" );
    moviec_write_size( _context, _writer, count );
    moviec_write_size( _context, _writer, json_array_size( _indices ) );
    json_t * arrays[2] = { _vertices, _uvs };

    for( size_t arrayIndex = 0; arrayIndex != 2; ++arrayIndex )
    {
        for( size_t index = 0; index != count; ++index )
        {
            moviec_write_point( _context, _writer, json_array_get( arrays[arrayIndex], index ) );
        }
    }

    for( size_t index = 0; index != json_array_size( _indices ); ++index )
    {
        float value = moviec_number( _context, json_array_get( _indices, index ), 0.f );
        moviec_require( _context, value >= 0.f && value < (float)count && floorf( value ) == value, "invalid mesh index" );
        moviec_write_byte( _context, _writer, (uint32_t)value & 255 );
        moviec_write_byte( _context, _writer, (uint32_t)value >> 8 );
    }
}
//////////////////////////////////////////////////////////////////////////
static json_t * moviec_geometry_points( MoviecContext * _context, json_t * _geometry, const char * _field, float _time )
{
    json_t * list = moviec_get( _geometry, "keys" );
    size_t count = json_array_size( list );

    if( count == 0 )
    {
        json_t * result = moviec_get( _geometry, _field );
        moviec_require( _context, json_is_array( result ), "geometry needs base %s", _field );

        return result;
    }

    size_t before = 0;
    float previous = -1.f;

    for( size_t index = 0; index != count; ++index )
    {
        json_t * key = json_array_get( list, index );
        float at = moviec_number( _context, moviec_get( key, "time" ), 0.f );
        moviec_require( _context, at >= 0.f && at > previous, "geometry keys must have strictly increasing times" );
        previous = at;
        moviec_require( _context, json_array_size( moviec_get( key, _field ) ) == json_array_size( moviec_get( _geometry, _field ) ), "geometry topology must remain constant" );

        if( at <= _time )
        {
            before = index;
        }
    }

    size_t after = before + 1 < count ? before + 1 : count - 1;
    json_t * firstKey = json_array_get( list, before );
    json_t * lastKey = json_array_get( list, after );
    float begin = moviec_number( _context, moviec_get( firstKey, "time" ), 0.f );
    float end = moviec_number( _context, moviec_get( lastKey, "time" ), 0.f );
    float weight = 0.f;

    if( end > begin && _time > begin )
    {
        aeMotionKey curve[2] = { { 0 } };
        curve[0].time = begin;
        curve[1].time = end;
        curve[1].value = 1.f;
        const char * interpolation = moviec_text( moviec_get( firstKey, "interpolation" ), "linear" );
        moviec_require( _context, strcmp( interpolation, "linear" ) == 0 || strcmp( interpolation, "hold" ) == 0 || strcmp( interpolation, "bezier" ) == 0 || strcmp( interpolation, "ease_in" ) == 0 || strcmp( interpolation, "ease_out" ) == 0 || strcmp( interpolation, "ease_in_out" ) == 0, "unsupported geometry interpolation" );
        curve[0].interpolation = strcmp( interpolation, "hold" ) == 0 ? 0 : strcmp( interpolation, "linear" ) == 0 ? 1
                                                                                                                   : 2;
        curve[0].out_time = begin + ( end - begin ) / 3.f;
        curve[1].in_time = end - ( end - begin ) / 3.f;
        curve[0].out_value = strcmp( interpolation, "ease_in" ) == 0 || strcmp( interpolation, "ease_in_out" ) == 0 ? 0.f : 1.f / 3.f;
        curve[1].in_value = strcmp( interpolation, "ease_out" ) == 0 || strcmp( interpolation, "ease_in_out" ) == 0 ? 1.f : 2.f / 3.f;
        json_t * out = moviec_get( firstKey, "out" );
        json_t * in = moviec_get( lastKey, "in" );

        if( out != NULL )
        {
            float values[2];
            moviec_vector( _context, out, 2, 0.f, values );
            curve[0].out_time = values[0];
            curve[0].out_value = values[1];
        }

        if( in != NULL )
        {
            float values[2];
            moviec_vector( _context, in, 2, 0.f, values );
            curve[1].in_time = values[0];
            curve[1].in_value = values[1];
        }

        moviec_require( _context, curve[0].out_time >= begin && curve[0].out_time <= end && curve[1].in_time >= begin && curve[1].in_time <= end, "invalid geometry Bezier handles" );
        weight = ae_movie_motion_sample_keys( curve, 2, _time );
    }

    json_t * result = moviec_array( _context );
    json_t * first = moviec_get( firstKey, _field );
    json_t * last = moviec_get( lastKey, _field );

    for( size_t index = 0; index != json_array_size( first ); ++index )
    {
        float from[2];
        float to[2];
        moviec_vector( _context, json_array_get( first, index ), 2, 0.f, from );
        moviec_vector( _context, json_array_get( last, index ), 2, 0.f, to );
        json_t * point = moviec_array( _context );
        moviec_json_append_number( _context, point, from[0] + ( to[0] - from[0] ) * weight );
        moviec_json_append_number( _context, point, from[1] + ( to[1] - from[1] ) * weight );
        moviec_json_append( _context, result, point );
    }

    return result;
}
//////////////////////////////////////////////////////////////////////////
void moviec_geometry( MoviecContext * _context, MoviecWriter * _writer, json_t * _layer, uint32_t _frames, float _fps, float _duration )
{
    json_t * mesh = moviec_get( _layer, "mesh" );
    json_t * warp = moviec_get( _layer, "warp" );
    moviec_require( _context, mesh == NULL || warp == NULL, "choose mesh or warp for a layer" );
    json_t * shapes[2] = { mesh, warp };

    for( size_t shapeIndex = 0; shapeIndex != 2; ++shapeIndex )
    {
        json_t * shape = shapes[shapeIndex];

        if( shape == NULL )
        {
            continue;
        }

        json_t * list = moviec_get( shape, "keys" );
        moviec_require( _context, list == NULL || json_is_array( list ), "geometry keys must be an array" );

        for( size_t index = 0; index != json_array_size( list ); ++index )
        {
            json_t * key = json_array_get( list, index );
            moviec_members( _context, key, "time vertices corners beziers interpolation in out" );
            moviec_needs( _context, key, "time" );
            moviec_require( _context, moviec_number( _context, moviec_get( key, "time" ), 0.f ) <= _duration, "geometry key outside composition" );
        }
    }

    if( mesh != NULL )
    {
        moviec_members( _context, mesh, "vertices uvs indices bakeFps keys" );
        int animated = json_array_size( moviec_get( mesh, "keys" ) ) > 0;
        moviec_write_byte( _context, _writer, 2 );
        moviec_write_byte( _context, _writer, animated == 0 );

        for( uint32_t frame = 0; frame != ( animated != 0 ? _frames : 1 ); ++frame )
        {
            size_t first = json_array_size( _context->ownedJson );
            float time = fminf( _duration, (float)frame / _fps );
            json_t * points = moviec_geometry_points( _context, mesh, "vertices", time );
            moviec_mesh( _context, _writer, points, moviec_get( mesh, "uvs" ), moviec_get( mesh, "indices" ) );
            moviec_release_geometry_points( _context, first );
        }

        moviec_write_byte( _context, _writer, 0 );
    }

    if( warp != NULL )
    {
        moviec_members( _context, warp, "corners beziers quality bakeFps keys" );
        moviec_require( _context, json_array_size( moviec_get( warp, "corners" ) ) == 4 && json_array_size( moviec_get( warp, "beziers" ) ) == 8, "warp needs four corners and eight handles" );
        int animated = json_array_size( moviec_get( warp, "keys" ) ) > 0;
        moviec_write_byte( _context, _writer, 3 );
        moviec_write_byte( _context, _writer, animated == 0 );
        static const char * const FIELDS[] = { "corners", "beziers" };

        for( uint32_t frame = 0; frame != ( animated != 0 ? _frames : 1 ); ++frame )
        {
            size_t first = json_array_size( _context->ownedJson );

            for( size_t field = 0; field != 2; ++field )
            {
                float time = fminf( _duration, (float)frame / _fps );
                json_t * points = moviec_geometry_points( _context, warp, FIELDS[field], time );

                for( size_t index = 0; index != json_array_size( points ); ++index )
                {
                    moviec_write_point( _context, _writer, json_array_get( points, index ) );
                }
            }

            moviec_release_geometry_points( _context, first );
        }

        float quality = moviec_number( _context, moviec_get( warp, "quality" ), 1.f );
        moviec_require( _context, quality >= 0.f && quality < AE_MOVIE_BEZIER_MAX_QUALITY && floorf( quality ) == quality, "invalid warp quality" );
        moviec_write_byte( _context, _writer, (uint32_t)quality );
        moviec_write_byte( _context, _writer, 0 );
    }
}
