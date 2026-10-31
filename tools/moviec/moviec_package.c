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

#include <lz4.h>
#include <stdlib.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
void * LZ4_malloc( size_t _size )
{
    void * memory = malloc( _size );

    return memory;
}
//////////////////////////////////////////////////////////////////////////
void * LZ4_calloc( size_t _count, size_t _size )
{
    void * memory = calloc( _count, _size );

    return memory;
}
//////////////////////////////////////////////////////////////////////////
void LZ4_free( void * _memory )
{
    free( _memory );
}
//////////////////////////////////////////////////////////////////////////
MoviecWriter moviec_unpack( MoviecContext * _context, MoviecWriter _input )
{
    if( _input.size < 4 || memcmp( _input.bytes, "AEZ1", 4 ) != 0 )
    {
        return _input;
    }

    moviec_require( _context, _input.size >= 20, "AEZ header is truncated" );
    uint32_t size = moviec_read_u32( _input.bytes + 12 );
    uint32_t compressed = moviec_read_u32( _input.bytes + 16 );
    moviec_require( _context, moviec_read_u32( _input.bytes + 4 ) == 1 && size <= 67108864 && compressed == _input.size - 20, "invalid AEZ header" );
    MoviecWriter result = { 0 };
    result.bytes = moviec_allocate( _context, size );
    result.size = size;
    int count = LZ4_decompress_safe( (const char *)_input.bytes + 20, (char *)result.bytes, (int)compressed, (int)size );
    moviec_require( _context, count == (int)size, "invalid AEZ payload" );

    return result;
}
//////////////////////////////////////////////////////////////////////////
MoviecWriter moviec_pack( MoviecContext * _context, MoviecWriter _input )
{
    int bound = LZ4_compressBound( (int)_input.size );
    char * compressed = moviec_allocate( _context, (size_t)bound );
    int count = LZ4_compress_default( (const char *)_input.bytes, compressed, (int)_input.size, bound );
    moviec_require( _context, count > 0, "LZ4 compression failed" );
    MoviecWriter result = { 0 };
    moviec_write_raw( _context, &result, "AEZ1", 4 );
    moviec_write_u32( _context, &result, 1 );
    moviec_write_u32( _context, &result, 0 );
    moviec_write_u32( _context, &result, (uint32_t)_input.size );
    moviec_write_u32( _context, &result, (uint32_t)count );
    moviec_write_raw( _context, &result, compressed, (size_t)count );

    return result;
}
//////////////////////////////////////////////////////////////////////////
static void moviec_xml_text( MoviecContext * _context, MoviecWriter * _writer, const char * _text )
{
    moviec_write_raw( _context, _writer, _text, strlen( _text ) );
}
//////////////////////////////////////////////////////////////////////////
static const char * moviec_xml_escape( MoviecContext * _context, const char * _value )
{
    MoviecWriter result = { 0 };

    for( const char * ch = _value; *ch != '\0'; ++ch )
    {
        if( *ch == '&' )
        {
            moviec_xml_text( _context, &result, "&amp;" );
        }
        else if( *ch == '<' )
        {
            moviec_xml_text( _context, &result, "&lt;" );
        }
        else if( *ch == '"' )
        {
            moviec_xml_text( _context, &result, "&quot;" );
        }
        else
        {
            moviec_write_byte( _context, &result, (unsigned char)*ch );
        }
    }

    moviec_write_byte( _context, &result, 0 );
    const char * text = (const char *)result.bytes;

    return text;
}
//////////////////////////////////////////////////////////////////////////
static const char * moviec_xml_vector( MoviecContext * _context, json_t * _value, size_t _dimension )
{
    float values[4];
    moviec_vector( _context, _value, _dimension, 0.f, values );
    MoviecWriter result = { 0 };

    for( size_t index = 0; index != _dimension; ++index )
    {
        const char * component = moviec_format( _context, index == 0 ? "%.6g" : ";%.6g", (double)values[index] );
        moviec_xml_text( _context, &result, component );
    }

    moviec_write_byte( _context, &result, 0 );
    const char * text = (const char *)result.bytes;

    return text;
}
//////////////////////////////////////////////////////////////////////////
static void moviec_xml_layers( MoviecContext * _context, MoviecWriter * _writer, json_t * _composition, size_t * _nodeIndex )
{
    json_t * layers = moviec_get( _composition, "layers" );
    json_t * assets = moviec_get( _context->document, "resources" );

    for( size_t index = 0; index != json_array_size( layers ); ++index )
    {
        json_t * layer = json_array_get( layers, index );
        const char * asset = moviec_text( moviec_get( layer, "resource" ), "" );
        const char * type = "Null";

        if( asset[0] != '\0' )
        {
            const char * resourceType = moviec_text( moviec_get( moviec_get( assets, asset ), "type" ), "image" );
            static const char * const NAMES[] = { "image", "sound", "particle", "video", "sequence", "solid" };
            static const char * const TYPES[] = { "Image", "Sound", "AstralaxEmitter", "Video", "Sequence", "Solid" };

            for( size_t typeIndex = 0; typeIndex != 6; ++typeIndex )
            {
                if( strcmp( resourceType, NAMES[typeIndex] ) == 0 )
                {
                    type = TYPES[typeIndex];
                    break;
                }
            }
        }

        const char * child = moviec_text( moviec_get( layer, "composition" ), "" );

        if( child[0] != '\0' )
        {
            type = "Movie";
        }

        if( strcmp( moviec_text( moviec_get( layer, "type" ), "" ), "slot" ) == 0 )
        {
            type = "Movie2Slot";
        }

        const char * id = moviec_xml_escape( _context, moviec_text( moviec_get( layer, "id" ), "" ) );
        const char * line = moviec_format( _context, "      <Layer Index=\"%zu\" Name=\"%s\" Type=\"%s\" TrackMatte=\"%d\" />\n", ( *_nodeIndex )++, id, type, json_is_true( moviec_get( layer, "isMatte" ) ) );
        moviec_xml_text( _context, _writer, line );

        if( child[0] != '\0' )
        {
            uint32_t childIndex = moviec_map_index( _context, _context->compositions, child );
            json_t * composition = json_array_get( moviec_get( _context->document, "compositions" ), childIndex );
            moviec_xml_layers( _context, _writer, composition, _nodeIndex );
        }
    }
}
//////////////////////////////////////////////////////////////////////////
void moviec_write_resources( MoviecContext * _context, const char * _output, const char * _packageRoot, int _absoluteAssets )
{
    MoviecWriter xml = { 0 };
    const char * name = moviec_text( moviec_get( _context->document, "name" ), moviec_path_stem( _context, _output ) );
    const char * escapedName = moviec_xml_escape( _context, name );
    moviec_xml_text( _context, &xml, moviec_format( _context, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<DataBlock Name=\"%s\">\n", escapedName ) );
    json_t * assets = moviec_get( _context->document, "resources" );

    for( size_t index = 0; index != json_array_size( _context->resourceNames ); ++index )
    {
        const char * id = json_string_value( json_array_get( _context->resourceNames, index ) );
        json_t * asset = moviec_get( assets, id );
        const char * type = moviec_text( moviec_get( asset, "type" ), "image" );

        if( strcmp( type, "solid" ) == 0 || strcmp( type, "sequence" ) == 0 )
        {
            continue;
        }

        const char * path = moviec_text( moviec_get( asset, "path" ), "" );

        if( _absoluteAssets != 0 && path[0] != '\0' )
        {
            path = moviec_path_absolute( _context, moviec_path_join( _context, _context->assetRoot, path ) );
        }

        const char * engineType = strcmp( type, "image" ) == 0 ? "ResourceImageDefault" : strcmp( type, "sound" ) == 0  ? "ResourceSound"
                                                                                      : strcmp( type, "particle" ) == 0 ? "ResourceAstralax"
                                                                                                                        : "ResourceVideo";
        moviec_xml_text( _context, &xml, moviec_format( _context, "  <Resource Name=\"%s\" Type=\"%s\">\n    <File Path=\"%s\"", moviec_xml_escape( _context, id ), engineType, moviec_xml_escape( _context, path ) ) );

        if( strcmp( type, "image" ) == 0 )
        {
            json_t * size = moviec_get( asset, "textureSize" );

            if( size == NULL )
            {
                size = moviec_get( asset, "size" );
            }

            moviec_xml_text( _context, &xml, moviec_format( _context, " MaxSize=\"%s\"", moviec_xml_vector( _context, size, 2 ) ) );
        }

        if( strcmp( type, "sound" ) == 0 )
        {
            const char * codec = strcmp( moviec_path_extension( path ), ".wav" ) == 0 ? "wavSound" : "oggSound";
            moviec_xml_text( _context, &xml, moviec_format( _context, " Codec=\"%s\"", codec ) );
        }

        if( strcmp( type, "video" ) == 0 )
        {
            float fps = moviec_number( _context, moviec_get( asset, "fps" ), 30.f );
            float duration = moviec_number( _context, moviec_get( asset, "duration" ), 0.f );
            moviec_xml_text( _context, &xml, moviec_format( _context, " Alpha=\"%d\" FrameRate=\"%.6g\" Duration=\"%.6g\"", json_is_true( moviec_get( asset, "alpha" ) ), (double)fps, (double)( duration * 1000.f ) ) );
        }

        moviec_xml_text( _context, &xml, " />\n" );

        if( strcmp( type, "sound" ) == 0 )
        {
            moviec_xml_text( _context, &xml, "    <DefaultVolume Value=\"1\" />\n" );
        }

        if( strcmp( type, "particle" ) == 0 )
        {
            json_t * images = moviec_get( asset, "images" );
            moviec_xml_text( _context, &xml, moviec_format( _context, "    <AtlasCount Value=\"%zu\" />\n", json_array_size( images ) ) );

            for( size_t image = 0; image != json_array_size( images ); ++image )
            {
                const char * imageName = moviec_text( json_array_get( images, image ), "" );
                moviec_xml_text( _context, &xml, moviec_format( _context, "    <Atlas Index=\"%zu\" ResourceName=\"%s\" />\n", image, moviec_xml_escape( _context, imageName ) ) );
            }
        }

        moviec_xml_text( _context, &xml, "  </Resource>\n" );
    }

    for( size_t index = 0; index != json_array_size( _context->resourceNames ); ++index )
    {
        const char * id = json_string_value( json_array_get( _context->resourceNames, index ) );
        json_t * asset = moviec_get( assets, id );
        json_t * uv = moviec_get( asset, "uv" );

        if( strcmp( moviec_text( moviec_get( asset, "type" ), "image" ), "image" ) != 0 || uv == NULL )
        {
            continue;
        }

        const char * coordinates = moviec_format( _context, "%s;%s;%s;%s", moviec_xml_vector( _context, json_array_get( uv, 0 ), 2 ), moviec_xml_vector( _context, json_array_get( uv, 1 ), 2 ), moviec_xml_vector( _context, json_array_get( uv, 2 ), 2 ), moviec_xml_vector( _context, json_array_get( uv, 3 ), 2 ) );
        const char * size = moviec_xml_vector( _context, moviec_get( asset, "size" ), 2 );
        const char * escapedId = moviec_xml_escape( _context, id );
        moviec_xml_text( _context, &xml, moviec_format( _context, "  <Resource Name=\"%s/sprite\" Type=\"ResourceImageSubstract\">\n    <Image Name=\"%s\" UV=\"%s\" MaxSize=\"%s\" Size=\"%s\" Offset=\"0;0\" Alpha=\"1\" />\n  </Resource>\n", escapedId, escapedId, coordinates, size, size ) );
    }

    const char * relative = _absoluteAssets != 0 ? moviec_path_absolute( _context, _output ) : _packageRoot[0] == '\0' ? moviec_path_filename( _output )
                                                                                                                       : moviec_path_relative( _context, _output, _packageRoot );
    moviec_xml_text( _context, &xml, moviec_format( _context, "  <Resource Name=\"Movie2_%s\" Type=\"ResourceMovie2\">\n    <File Path=\"%s\" />\n", escapedName, moviec_xml_escape( _context, relative ) ) );
    json_t * list = moviec_get( _context->document, "compositions" );

    for( size_t index = 0; index != json_array_size( list ); ++index )
    {
        json_t * composition = json_array_get( list, index );
        float size[2];
        moviec_vector( _context, moviec_get( composition, "size" ), 2, 0.f, size );
        const char * id = moviec_xml_escape( _context, moviec_text( moviec_get( composition, "id" ), "" ) );
        float duration = moviec_number( _context, moviec_get( composition, "duration" ), 0.f );
        float fps = moviec_effective_fps( _context, composition );
        moviec_xml_text( _context, &xml, moviec_format( _context, "    <Composition Name=\"%s\" Duration=\"%.6g\" FrameDuration=\"%.6g\" Master=\"1\" Bounds=\"0;0;%.6g;%.6g\">\n", id, (double)duration, (double)( 1.f / fps ), (double)size[0], (double)size[1] ) );
        size_t nodeIndex = 0;
        moviec_xml_layers( _context, &xml, composition, &nodeIndex );
        moviec_xml_text( _context, &xml, "    </Composition>\n" );
    }

    moviec_xml_text( _context, &xml, "  </Resource>\n</DataBlock>\n" );
    moviec_write_file( _context, moviec_format( _context, "%s.xml", _output ), xml.bytes, xml.size );
}
