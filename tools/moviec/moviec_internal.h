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

#ifndef MOVIEC_INTERNAL_H_
#define MOVIEC_INTERNAL_H_

#include "moviec.h"
#include "movie/movie.h"
#include "movie_motion_format.h"

#include <jansson.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>

//////////////////////////////////////////////////////////////////////////
typedef struct MoviecAllocation
{
    struct MoviecAllocation * next;
    size_t size;
} MoviecAllocation;
//////////////////////////////////////////////////////////////////////////
typedef struct MoviecWriter
{
    uint8_t * bytes;
    size_t size;
    size_t capacity;
} MoviecWriter;
//////////////////////////////////////////////////////////////////////////
typedef struct MoviecCode
{
    aeMotionInstruction * instructions;
    size_t count;
    size_t capacity;
} MoviecCode;
//////////////////////////////////////////////////////////////////////////
typedef struct MoviecTrack
{
    const char * name;
    const char * expression;
    uint32_t composition;
    uint32_t layer;
    uint32_t property;
    uint32_t extrapolation;
    float value;
    aeMotionKey * keys;
    size_t keyCount;
    MoviecCode code;
    uint32_t base[4];
    size_t dimension;
    size_t component;
} MoviecTrack;
//////////////////////////////////////////////////////////////////////////
typedef struct MoviecContext
{
    jmp_buf errorJump;
    MoviecAllocation * allocations;
    json_t * ownedJson;
    const char * path;
    const char * source;
    char error[1024];

    json_t * document;
    json_t * parameters;
    json_t * parameterNames;
    json_t * resources;
    json_t * resourceNames;
    json_t * compositions;
    json_t * channels;
    json_t * manifest;
    json_t * sourceMap;
    const char * assetRoot;

    MoviecTrack * tracks;
    size_t trackCount;
    size_t trackCapacity;

    const aeMovieInstance * instance;
    aeMovieData * movieData;
    const aeMovieComposition * composition;
    uint32_t major;
    uint32_t minor;
} MoviecContext;
//////////////////////////////////////////////////////////////////////////
_Noreturn void moviec_fail( MoviecContext * _context, const char * _format, ... );
void moviec_require( MoviecContext * _context, int _condition, const char * _format, ... );
void * moviec_allocate( MoviecContext * _context, size_t _size );
void * moviec_grow( MoviecContext * _context, const void * _old, size_t _oldSize, size_t _newSize );
char * moviec_format( MoviecContext * _context, const char * _format, ... );
char * moviec_string_copy( MoviecContext * _context, const char * _value, size_t _size );
void moviec_finalize( MoviecContext * _context );

json_t * moviec_own_json( MoviecContext * _context, json_t * _value );
json_t * moviec_object( MoviecContext * _context );
json_t * moviec_array( MoviecContext * _context );
json_t * moviec_get( const json_t * _object, const char * _key );
const char * moviec_text( const json_t * _value, const char * _fallback );
float moviec_number( MoviecContext * _context, const json_t * _value, float _fallback );
void moviec_vector( MoviecContext * _context, const json_t * _value, size_t _dimension, float _fallback, float * _out );
json_t * moviec_keys( MoviecContext * _context, json_t * _object );
void moviec_members( MoviecContext * _context, json_t * _object, const char * _allowed );
void moviec_needs( MoviecContext * _context, json_t * _object, const char * _required );
void moviec_json_set( MoviecContext * _context, json_t * _object, const char * _key, json_t * _value );
void moviec_json_string( MoviecContext * _context, json_t * _object, const char * _key, const char * _value );
void moviec_json_number( MoviecContext * _context, json_t * _object, const char * _key, double _value );
void moviec_json_integer( MoviecContext * _context, json_t * _object, const char * _key, json_int_t _value );
void moviec_json_append( MoviecContext * _context, json_t * _array, json_t * _value );
void moviec_json_append_string( MoviecContext * _context, json_t * _array, const char * _value );
void moviec_json_append_integer( MoviecContext * _context, json_t * _array, json_int_t _value );
void moviec_json_append_number( MoviecContext * _context, json_t * _array, double _value );
int moviec_map_find( json_t * _map, const char * _name, uint32_t * _index );
uint32_t moviec_map_index( MoviecContext * _context, json_t * _map, const char * _name );

char * moviec_path_absolute( MoviecContext * _context, const char * _path );
char * moviec_path_directory( MoviecContext * _context, const char * _path );
char * moviec_path_join( MoviecContext * _context, const char * _base, const char * _path );
char * moviec_path_relative( MoviecContext * _context, const char * _path, const char * _base );
const char * moviec_path_filename( const char * _path );
const char * moviec_path_extension( const char * _path );
char * moviec_path_stem( MoviecContext * _context, const char * _path );
int moviec_file_exists( const char * _path );
void moviec_create_directories( MoviecContext * _context, const char * _path );
MoviecWriter moviec_read_file( MoviecContext * _context, const char * _path );
void moviec_write_file( MoviecContext * _context, const char * _path, const void * _bytes, size_t _size );
void moviec_save_json( MoviecContext * _context, const char * _path, json_t * _value );

void moviec_write_raw( MoviecContext * _context, MoviecWriter * _writer, const void * _bytes, size_t _size );
void moviec_write_byte( MoviecContext * _context, MoviecWriter * _writer, uint32_t _value );
void moviec_write_u32( MoviecContext * _context, MoviecWriter * _writer, uint32_t _value );
void moviec_write_float( MoviecContext * _context, MoviecWriter * _writer, float _value );
void moviec_write_string( MoviecContext * _context, MoviecWriter * _writer, const char * _value );
void moviec_write_size( MoviecContext * _context, MoviecWriter * _writer, size_t _value );
void moviec_write_legacy_string( MoviecContext * _context, MoviecWriter * _writer, const char * _value );
uint32_t moviec_read_u32( const uint8_t * _bytes );

float moviec_effective_fps( MoviecContext * _context, json_t * _composition );
uint32_t moviec_resource_type( MoviecContext * _context, json_t * _resource );
void moviec_compile_expression( MoviecContext * _context, MoviecTrack * _track );
void moviec_collect( MoviecContext * _context );
MoviecWriter moviec_compile( MoviecContext * _context );
MoviecWriter moviec_scaffold( MoviecContext * _context );
void moviec_geometry( MoviecContext * _context, MoviecWriter * _writer, json_t * _layer, uint32_t _frames, float _fps, float _duration );
MoviecWriter moviec_unpack( MoviecContext * _context, MoviecWriter _input );
MoviecWriter moviec_pack( MoviecContext * _context, MoviecWriter _input );
void moviec_write_resources( MoviecContext * _context, const char * _output, const char * _packageRoot, int _absoluteAssets );
void moviec_load( MoviecContext * _context, const MoviecWriter * _input, const char * _key );

#endif
