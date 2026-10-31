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
static void moviec_write_vector( MoviecContext * _context, MoviecWriter * _writer, json_t * _value, size_t _dimension, float _fallback )
{
    float values[4];
    moviec_vector( _context, _value, _dimension, _fallback, values );

    for( size_t index = 0; index != _dimension; ++index )
    {
        moviec_write_float( _context, _writer, values[index] );
    }
}
//////////////////////////////////////////////////////////////////////////
static void moviec_resource( MoviecContext * _context, MoviecWriter * _writer, const char * _id )
{
    _context->path = moviec_format( _context, "/resources/%s", _id );
    json_t * data = moviec_get( moviec_get( _context->document, "resources" ), _id );
    uint32_t type = moviec_resource_type( _context, data );
    moviec_write_byte( _context, _writer, type );
    moviec_write_legacy_string( _context, _writer, _id );
    const char * path = moviec_text( moviec_get( data, "path" ), "" );

    if( type == AE_MOVIE_RESOURCE_IMAGE || type == AE_MOVIE_RESOURCE_SOUND || type == AE_MOVIE_RESOURCE_VIDEO || type == AE_MOVIE_RESOURCE_PARTICLE )
    {
        moviec_require( _context, path[0] != '\0', "resource needs a path" );
        const char * asset = moviec_path_join( _context, _context->assetRoot, path );
        moviec_require( _context, moviec_file_exists( asset ), "missing resource: %s", asset );
        moviec_write_legacy_string( _context, _writer, path );
        moviec_write_byte( _context, _writer, 0 );
    }

    if( type == AE_MOVIE_RESOURCE_IMAGE )
    {
        float size[2];
        moviec_vector( _context, moviec_get( data, "size" ), 2, 0.f, size );
        moviec_require( _context, size[0] > 0.f && size[1] > 0.f, "image needs positive size" );
        moviec_write_u32( _context, _writer, 0 );
        moviec_write_float( _context, _writer, size[0] );
        moviec_write_float( _context, _writer, size[1] );
        json_t * trim = moviec_get( data, "trim" );

        if( trim != NULL )
        {
            moviec_write_byte( _context, _writer, 1 );
            moviec_write_vector( _context, _writer, trim, 4, 0.f );
        }

        json_t * uv = moviec_get( data, "uv" );

        if( uv != NULL )
        {
            moviec_require( _context, json_is_array( uv ) && json_array_size( uv ) == 4, "uv needs four corners" );
            moviec_write_byte( _context, _writer, 2 );

            for( size_t index = 0; index != 4; ++index )
            {
                moviec_write_vector( _context, _writer, json_array_get( uv, index ), 2, 0.f );
            }
        }

        moviec_write_byte( _context, _writer, 0 );
    }
    else if( type == AE_MOVIE_RESOURCE_SOUND )
    {
        float duration = moviec_number( _context, moviec_get( data, "duration" ), 0.f );
        moviec_require( _context, duration > 0.f, "sound needs positive duration" );
        moviec_write_float( _context, _writer, duration );
        moviec_write_byte( _context, _writer, 0 );
    }
    else if( type == AE_MOVIE_RESOURCE_PARTICLE || type == AE_MOVIE_RESOURCE_SEQUENCE )
    {
        json_t * images = moviec_get( data, "images" );

        if( type == AE_MOVIE_RESOURCE_SEQUENCE )
        {
            float fps = moviec_number( _context, moviec_get( data, "fps" ), 30.f );
            moviec_require( _context, fps > 0.f, "invalid sequence fps" );
            moviec_require( _context, json_is_array( images ) && json_array_size( images ) > 0, "sequence needs images" );
            moviec_write_float( _context, _writer, fps );
        }

        moviec_write_size( _context, _writer, json_array_size( images ) );

        for( size_t index = 0; index != json_array_size( images ); ++index )
        {
            const char * image = moviec_text( json_array_get( images, index ), "" );
            uint32_t resourceIndex = moviec_map_index( _context, _context->resources, image );
            json_t * resource = moviec_get( moviec_get( _context->document, "resources" ), image );
            moviec_require( _context, moviec_resource_type( _context, resource ) == AE_MOVIE_RESOURCE_IMAGE, "image resource not found: %s", image );
            moviec_require( _context, resourceIndex < moviec_map_index( _context, _context->resources, _id ), "image resource must precede its user" );
            moviec_write_size( _context, _writer, resourceIndex );
        }

        moviec_write_byte( _context, _writer, 0 );
    }
    else if( type == AE_MOVIE_RESOURCE_SOLID )
    {
        moviec_write_vector( _context, _writer, moviec_get( data, "size" ), 2, 0.f );
        moviec_write_vector( _context, _writer, moviec_get( data, "color" ), 3, 1.f );
        moviec_write_byte( _context, _writer, 0 );
    }
    else if( type == AE_MOVIE_RESOURCE_VIDEO )
    {
        float duration = moviec_number( _context, moviec_get( data, "duration" ), 0.f );
        float fps = moviec_number( _context, moviec_get( data, "fps" ), 30.f );
        moviec_require( _context, duration > 0.f && fps > 0.f, "invalid video timing" );
        moviec_write_vector( _context, _writer, moviec_get( data, "size" ), 2, 0.f );
        moviec_write_byte( _context, _writer, json_is_true( moviec_get( data, "alpha" ) ) );
        moviec_write_float( _context, _writer, fps );
        moviec_write_float( _context, _writer, duration );
        moviec_write_byte( _context, _writer, 0 );
    }

    moviec_json_set( _context, _context->manifest, _id, data );
}
//////////////////////////////////////////////////////////////////////////
static uint32_t moviec_layer_type( MoviecContext * _context, json_t * _layer )
{
    const char * child = moviec_text( moviec_get( _layer, "composition" ), "" );

    if( child[0] != '\0' )
    {
        return AE_MOVIE_LAYER_TYPE_MOVIE;
    }

    const char * asset = moviec_text( moviec_get( _layer, "resource" ), "" );

    if( asset[0] != '\0' )
    {
        json_t * resource = moviec_get( moviec_get( _context->document, "resources" ), asset );
        uint32_t type = moviec_resource_type( _context, resource );

        switch( type )
        {
        case AE_MOVIE_RESOURCE_IMAGE:
            {
                return AE_MOVIE_LAYER_TYPE_IMAGE;
            }break;
        case AE_MOVIE_RESOURCE_SOUND:
            {
                return AE_MOVIE_LAYER_TYPE_SOUND;
            }break;
        case AE_MOVIE_RESOURCE_PARTICLE:
            {
                return AE_MOVIE_LAYER_TYPE_PARTICLE;
            }break;
        case AE_MOVIE_RESOURCE_VIDEO:
            {
                return AE_MOVIE_LAYER_TYPE_VIDEO;
            }break;
        case AE_MOVIE_RESOURCE_SEQUENCE:
            {
                return AE_MOVIE_LAYER_TYPE_SEQUENCE;
            }break;
        case AE_MOVIE_RESOURCE_SOLID:
            {
                return AE_MOVIE_LAYER_TYPE_SOLID;
            }break;
        default:
            {
                moviec_fail( _context, "unsupported layer resource" );
            }break;
        }
    }

    const char * type = moviec_text( moviec_get( _layer, "type" ), "" );

    if( strcmp( type, "slot" ) == 0 )
    {
        return AE_MOVIE_LAYER_TYPE_SLOT;
    }

    moviec_require( _context, type[0] == '\0' || strcmp( type, "null" ) == 0, "unsupported layer type: %s", type );

    return AE_MOVIE_LAYER_TYPE_NULL;
}
//////////////////////////////////////////////////////////////////////////
static void moviec_layer( MoviecContext * _context, MoviecWriter * _writer, json_t * _layers, size_t _index, json_t * _ids, float _duration, float _fps )
{
    json_t * layer = json_array_get( _layers, _index );
    moviec_write_legacy_string( _context, _writer, moviec_text( moviec_get( layer, "id" ), "" ) );
    moviec_write_size( _context, _writer, _index + 1 );
    moviec_write_byte( _context, _writer, json_is_true( moviec_get( layer, "isMatte" ) ) );
    const char * matte = moviec_text( moviec_get( layer, "matte" ), "" );
    moviec_write_byte( _context, _writer, matte[0] != '\0' );

    if( matte[0] != '\0' )
    {
        json_t * source = json_array_get( _layers, _index + 1 );
        const char * sourceId = moviec_text( moviec_get( source, "id" ), "" );
        moviec_require( _context, source != NULL && strcmp( sourceId, matte ) == 0, "matte must immediately follow its target in the layers array" );
        const char * sourceAsset = moviec_text( moviec_get( source, "resource" ), "" );
        json_t * resource = moviec_get( moviec_get( _context->document, "resources" ), sourceAsset );
        moviec_require( _context, json_is_true( moviec_get( source, "isMatte" ) ) && resource != NULL && moviec_resource_type( _context, resource ) == AE_MOVIE_RESOURCE_IMAGE, "matte source must be an image layer with isMatte: true" );
        const char * mode = moviec_text( moviec_get( layer, "matteMode" ), "alpha" );
        static const char * const MODES[] = { "alpha", "alpha_inverted", "luma", "luma_inverted" };
        static const uint32_t VALUES[] = { AE_MOVIE_TRACK_MATTE_ALPHA, AE_MOVIE_TRACK_MATTE_ALPHA_INVERTED, AE_MOVIE_TRACK_MATTE_LUMA, AE_MOVIE_TRACK_MATTE_LUMA_INVERTED };
        size_t index = 0;

        for( ; index != 4; ++index )
        {
            if( strcmp( mode, MODES[index] ) == 0 )
            {
                break;
            }
        }

        moviec_require( _context, index != 4, "unknown matte mode" );
        moviec_write_byte( _context, _writer, VALUES[index] );
    }

    uint32_t type = moviec_layer_type( _context, layer );
    moviec_write_byte( _context, _writer, type );
    uint32_t frames = (uint32_t)ceilf( _duration * _fps ) + 2;
    moviec_write_size( _context, _writer, frames );
    moviec_geometry( _context, _writer, layer, frames, _fps, _duration );
    moviec_write_byte( _context, _writer, 0 );
    const char * child = moviec_text( moviec_get( layer, "composition" ), "" );
    const char * asset = moviec_text( moviec_get( layer, "resource" ), "" );
    uint32_t reference = 0;

    if( child[0] != '\0' )
    {
        reference = moviec_map_index( _context, _context->compositions, child );
    }
    else if( asset[0] != '\0' )
    {
        reference = moviec_map_index( _context, _context->resources, asset ) + 1;
    }

    moviec_write_byte( _context, _writer, child[0] == '\0' );
    moviec_write_size( _context, _writer, reference );
    const char * parent = moviec_text( moviec_get( layer, "parent" ), "" );
    moviec_write_size( _context, _writer, parent[0] == '\0' ? 0 : moviec_map_index( _context, _ids, parent ) );
    moviec_write_float( _context, _writer, moviec_number( _context, moviec_get( layer, "in" ), 0.f ) );
    moviec_write_float( _context, _writer, moviec_number( _context, moviec_get( layer, "out" ), _duration ) );
    moviec_write_float( _context, _writer, 0.f );
    moviec_write_float( _context, _writer, _duration );
    moviec_write_byte( _context, _writer, 0 );
    moviec_write_byte( _context, _writer, 0 );
    const char * blend = moviec_text( moviec_get( layer, "blend" ), "normal" );
    static const char * const BLENDS[] = { "normal", "add", "screen", "multiply" };
    static const uint32_t VALUES[] = { AE_MOVIE_BLEND_NORMAL, AE_MOVIE_BLEND_ADD, AE_MOVIE_BLEND_SCREEN, AE_MOVIE_BLEND_MULTIPLY };
    size_t blendIndex = 0;

    for( ; blendIndex != 4; ++blendIndex )
    {
        if( strcmp( blend, BLENDS[blendIndex] ) == 0 )
        {
            break;
        }
    }

    moviec_require( _context, blendIndex != 4, "unsupported blend mode" );
    moviec_write_byte( _context, _writer, VALUES[blendIndex] );
    moviec_write_byte( _context, _writer, 0 );

    if( json_is_true( moviec_get( layer, "loop" ) ) )
    {
        moviec_write_u32( _context, _writer, AE_OPTION( 'l', 'o', 'o', 'p' ) );
    }

    moviec_write_u32( _context, _writer, 0 );
    moviec_write_size( _context, _writer, 1 );
    moviec_write_float( _context, _writer, 1.f );
    moviec_write_u32( _context, _writer, 0xffffffffU );
    moviec_write_u32( _context, _writer, 0xffffffffU );
}
//////////////////////////////////////////////////////////////////////////
MoviecWriter moviec_scaffold( MoviecContext * _context )
{
    MoviecWriter writer = { 0 };
    moviec_write_raw( _context, &writer, "AEM1", 4 );
    moviec_write_u32( _context, &writer, AE_MOVIE_SDK_MAJOR_VERSION );
    moviec_write_u32( _context, &writer, AE_MOVIE_SDK_MINOR_VERSION );
    moviec_write_u32( _context, &writer, 0 );
    moviec_write_legacy_string( _context, &writer, moviec_text( moviec_get( _context->document, "name" ), "movie" ) );
    moviec_write_byte( _context, &writer, 0 );
    moviec_write_size( _context, &writer, 0 );
    moviec_write_size( _context, &writer, json_array_size( _context->resourceNames ) );

    for( size_t index = 0; index != json_array_size( _context->resourceNames ); ++index )
    {
        const char * name = json_string_value( json_array_get( _context->resourceNames, index ) );
        moviec_resource( _context, &writer, name );
    }

    json_t * list = moviec_get( _context->document, "compositions" );
    moviec_write_size( _context, &writer, json_array_size( list ) );

    for( size_t compositionIndex = 0; compositionIndex != json_array_size( list ); ++compositionIndex )
    {
        json_t * composition = json_array_get( list, compositionIndex );
        _context->path = moviec_format( _context, "/compositions/%zu", compositionIndex );
        float duration = moviec_number( _context, moviec_get( composition, "duration" ), 0.f );
        float fps = moviec_effective_fps( _context, composition );
        json_t * layers = moviec_get( composition, "layers" );
        moviec_write_legacy_string( _context, &writer, moviec_text( moviec_get( composition, "id" ), "" ) );
        moviec_write_byte( _context, &writer, 1 );
        moviec_write_vector( _context, &writer, moviec_get( composition, "size" ), 2, 0.f );
        moviec_write_float( _context, &writer, duration );
        moviec_write_float( _context, &writer, 1.f / fps );
        moviec_write_float( _context, &writer, fps );
        moviec_write_byte( _context, &writer, 4 );
        moviec_write_float( _context, &writer, 0.f );
        moviec_write_float( _context, &writer, 0.f );
        moviec_write_vector( _context, &writer, moviec_get( composition, "size" ), 2, 0.f );
        moviec_write_byte( _context, &writer, 0 );
        moviec_write_size( _context, &writer, json_array_size( layers ) );
        json_t * ids = moviec_object( _context );

        for( size_t index = 0; index != json_array_size( layers ); ++index )
        {
            const char * id = moviec_text( moviec_get( json_array_get( layers, index ), "id" ), "" );
            moviec_json_integer( _context, ids, id, (json_int_t)index + 1 );
        }

        for( size_t index = 0; index != json_array_size( layers ); ++index )
        {
            _context->path = moviec_format( _context, "/compositions/%zu/layers/%zu", compositionIndex, index );
            moviec_layer( _context, &writer, layers, index, ids, duration, fps );
        }
    }

    return writer;
}
