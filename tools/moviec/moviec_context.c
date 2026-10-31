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

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined( _WIN32 )
#include <direct.h>
#define MOVIEC_GETCWD _getcwd
#define MOVIEC_MKDIR( Path ) _mkdir( Path )
#else
#include <unistd.h>
#define MOVIEC_GETCWD getcwd
#define MOVIEC_MKDIR( Path ) mkdir( Path, 0777 )
#endif

//////////////////////////////////////////////////////////////////////////
_Noreturn void moviec_fail( MoviecContext * _context, const char * _format, ... )
{
    va_list args;
    va_start( args, _format );
    vsnprintf( _context->error, sizeof( _context->error ), _format, args );
    va_end( args );

    longjmp( _context->errorJump, 1 );
}
//////////////////////////////////////////////////////////////////////////
void moviec_require( MoviecContext * _context, int _condition, const char * _format, ... )
{
    if( _condition != 0 )
    {
        return;
    }

    va_list args;
    va_start( args, _format );
    vsnprintf( _context->error, sizeof( _context->error ), _format, args );
    va_end( args );

    longjmp( _context->errorJump, 1 );
}
//////////////////////////////////////////////////////////////////////////
void * moviec_allocate( MoviecContext * _context, size_t _size )
{
    if( _size > SIZE_MAX - sizeof( MoviecAllocation ) )
    {
        moviec_fail( _context, "allocation size overflow" );
    }

    MoviecAllocation * allocation = (MoviecAllocation *)calloc( 1, sizeof( MoviecAllocation ) + _size );

    if( allocation == NULL )
    {
        moviec_fail( _context, "out of memory" );
    }

    allocation->next = _context->allocations;
    allocation->size = _size;
    _context->allocations = allocation;

    void * memory = allocation + 1;

    return memory;
}
//////////////////////////////////////////////////////////////////////////
void * moviec_grow( MoviecContext * _context, const void * _old, size_t _oldSize, size_t _newSize )
{
    void * memory = moviec_allocate( _context, _newSize );

    if( _oldSize != 0 )
    {
        memcpy( memory, _old, _oldSize );
    }

    return memory;
}
//////////////////////////////////////////////////////////////////////////
char * moviec_format( MoviecContext * _context, const char * _format, ... )
{
    va_list args;
    va_list copy;
    va_start( args, _format );
    va_copy( copy, args );
    int size = vsnprintf( NULL, 0, _format, copy );
    va_end( copy );

    if( size < 0 )
    {
        va_end( args );
        moviec_fail( _context, "cannot format string" );
    }

    char * result = (char *)moviec_allocate( _context, (size_t)size + 1 );
    vsnprintf( result, (size_t)size + 1, _format, args );
    va_end( args );

    return result;
}
//////////////////////////////////////////////////////////////////////////
char * moviec_string_copy( MoviecContext * _context, const char * _value, size_t _size )
{
    char * result = (char *)moviec_allocate( _context, _size + 1 );
    memcpy( result, _value, _size );
    result[_size] = '\0';

    return result;
}
//////////////////////////////////////////////////////////////////////////
void moviec_finalize( MoviecContext * _context )
{
    if( _context->composition != NULL )
    {
        ae_delete_movie_composition( _context->composition );
    }

    if( _context->movieData != NULL )
    {
        ae_delete_movie_data( _context->movieData );
    }

    if( _context->instance != NULL )
    {
        ae_delete_movie_instance( _context->instance );
    }

    json_decref( _context->ownedJson );

    MoviecAllocation * allocation = _context->allocations;

    while( allocation != NULL )
    {
        MoviecAllocation * next = allocation->next;
        free( allocation );
        allocation = next;
    }
}
//////////////////////////////////////////////////////////////////////////
json_t * moviec_own_json( MoviecContext * _context, json_t * _value )
{
    if( _value == NULL )
    {
        moviec_fail( _context, "cannot allocate JSON value" );
    }

    if( json_array_append_new( _context->ownedJson, _value ) != 0 )
    {
        moviec_fail( _context, "cannot retain JSON value" );
    }

    return _value;
}
//////////////////////////////////////////////////////////////////////////
json_t * moviec_object( MoviecContext * _context )
{
    json_t * result = moviec_own_json( _context, json_object() );

    return result;
}
//////////////////////////////////////////////////////////////////////////
json_t * moviec_array( MoviecContext * _context )
{
    json_t * result = moviec_own_json( _context, json_array() );

    return result;
}
//////////////////////////////////////////////////////////////////////////
json_t * moviec_get( const json_t * _object, const char * _key )
{
    json_t * result = json_object_get( _object, _key );

    return result;
}
//////////////////////////////////////////////////////////////////////////
const char * moviec_text( const json_t * _value, const char * _fallback )
{
    if( json_is_string( _value ) == 0 )
    {
        return _fallback;
    }

    const char * result = json_string_value( _value );

    return result;
}
//////////////////////////////////////////////////////////////////////////
float moviec_number( MoviecContext * _context, const json_t * _value, float _fallback )
{
    if( _value == NULL )
    {
        return _fallback;
    }

    moviec_require( _context, json_is_number( _value ), "expected a number" );
    double number = json_number_value( _value );
    moviec_require( _context, isfinite( number ), "number is not finite" );
    moviec_require( _context, fabs( number ) <= 1.e20, "number is too large" );
    float result = (float)number;

    return result;
}
//////////////////////////////////////////////////////////////////////////
void moviec_vector( MoviecContext * _context, const json_t * _value, size_t _dimension, float _fallback, float * _out )
{
    if( json_is_array( _value ) == 0 )
    {
        float value = moviec_number( _context, _value, _fallback );

        for( size_t index = 0; index != _dimension; ++index )
        {
            _out[index] = value;
        }

        return;
    }

    moviec_require( _context, json_array_size( _value ) == _dimension, "vector dimension mismatch" );

    for( size_t index = 0; index != _dimension; ++index )
    {
        _out[index] = moviec_number( _context, json_array_get( _value, index ), 0.f );
    }
}
//////////////////////////////////////////////////////////////////////////
static int moviec_compare_keys( const void * _left, const void * _right )
{
    const char * left = *(const char * const *)_left;
    const char * right = *(const char * const *)_right;
    int result = strcmp( left, right );

    return result;
}
//////////////////////////////////////////////////////////////////////////
json_t * moviec_keys( MoviecContext * _context, json_t * _object )
{
    json_t * result = moviec_array( _context );

    if( _object == NULL )
    {
        return result;
    }

    moviec_require( _context, json_is_object( _object ), "expected an object" );
    size_t count = json_object_size( _object );
    const char ** names = (const char **)moviec_allocate( _context, count * sizeof( const char * ) );
    size_t index = 0;
    const char * key;
    json_t * value;
    json_object_foreach( _object, key, value )
    {
        (void)value;
        names[index++] = key;
    }

    qsort( names, count, sizeof( const char * ), moviec_compare_keys );

    for( index = 0; index != count; ++index )
    {
        moviec_json_append_string( _context, result, names[index] );
    }

    return result;
}
//////////////////////////////////////////////////////////////////////////
static int moviec_has_word( const char * _words, const char * _word )
{
    size_t length = strlen( _word );
    const char * cursor = _words;

    while( *cursor != '\0' )
    {
        const char * end = strchr( cursor, ' ' );
        size_t size = end != NULL ? (size_t)( end - cursor ) : strlen( cursor );

        if( size == length && memcmp( cursor, _word, length ) == 0 )
        {
            return 1;
        }

        if( end == NULL )
        {
            break;
        }

        cursor = end + 1;
    }

    return 0;
}
//////////////////////////////////////////////////////////////////////////
void moviec_members( MoviecContext * _context, json_t * _object, const char * _allowed )
{
    moviec_require( _context, json_is_object( _object ), "expected an object" );
    const char * key;
    json_t * value;
    json_object_foreach( _object, key, value )
    {
        (void)value;
        moviec_require( _context, moviec_has_word( _allowed, key ), "unknown field: %s", key );
    }
}
//////////////////////////////////////////////////////////////////////////
void moviec_needs( MoviecContext * _context, json_t * _object, const char * _required )
{
    const char * cursor = _required;

    while( *cursor != '\0' )
    {
        const char * end = strchr( cursor, ' ' );
        size_t length = end != NULL ? (size_t)( end - cursor ) : strlen( cursor );
        const char * key = moviec_string_copy( _context, cursor, length );
        moviec_require( _context, moviec_get( _object, key ) != NULL, "missing field: %s", key );

        if( end == NULL )
        {
            break;
        }

        cursor = end + 1;
    }
}
//////////////////////////////////////////////////////////////////////////
void moviec_json_set( MoviecContext * _context, json_t * _object, const char * _key, json_t * _value )
{
    moviec_require( _context, json_object_set( _object, _key, _value ) == 0, "cannot set JSON field" );
}
//////////////////////////////////////////////////////////////////////////
void moviec_json_string( MoviecContext * _context, json_t * _object, const char * _key, const char * _value )
{
    moviec_require( _context, json_object_set_new( _object, _key, json_string( _value ) ) == 0, "cannot set JSON string" );
}
//////////////////////////////////////////////////////////////////////////
void moviec_json_number( MoviecContext * _context, json_t * _object, const char * _key, double _value )
{
    moviec_require( _context, json_object_set_new( _object, _key, json_real( _value ) ) == 0, "cannot set JSON number" );
}
//////////////////////////////////////////////////////////////////////////
void moviec_json_integer( MoviecContext * _context, json_t * _object, const char * _key, json_int_t _value )
{
    moviec_require( _context, json_object_set_new( _object, _key, json_integer( _value ) ) == 0, "cannot set JSON integer" );
}
//////////////////////////////////////////////////////////////////////////
void moviec_json_append( MoviecContext * _context, json_t * _array, json_t * _value )
{
    moviec_require( _context, json_array_append( _array, _value ) == 0, "cannot append JSON value" );
}
//////////////////////////////////////////////////////////////////////////
void moviec_json_append_string( MoviecContext * _context, json_t * _array, const char * _value )
{
    moviec_require( _context, json_array_append_new( _array, json_string( _value ) ) == 0, "cannot append JSON string" );
}
//////////////////////////////////////////////////////////////////////////
void moviec_json_append_integer( MoviecContext * _context, json_t * _array, json_int_t _value )
{
    moviec_require( _context, json_array_append_new( _array, json_integer( _value ) ) == 0, "cannot append JSON integer" );
}
//////////////////////////////////////////////////////////////////////////
void moviec_json_append_number( MoviecContext * _context, json_t * _array, double _value )
{
    moviec_require( _context, json_array_append_new( _array, json_real( _value ) ) == 0, "cannot append JSON number" );
}
//////////////////////////////////////////////////////////////////////////
int moviec_map_find( json_t * _map, const char * _name, uint32_t * _index )
{
    json_t * value = moviec_get( _map, _name );

    if( value == NULL )
    {
        return 0;
    }

    *_index = (uint32_t)json_integer_value( value );

    return 1;
}
//////////////////////////////////////////////////////////////////////////
uint32_t moviec_map_index( MoviecContext * _context, json_t * _map, const char * _name )
{
    uint32_t index;
    moviec_require( _context, moviec_map_find( _map, _name, &index ), "unknown name: %s", _name );

    return index;
}
//////////////////////////////////////////////////////////////////////////
const char * moviec_path_filename( const char * _path )
{
    const char * result = _path;

    for( const char * cursor = _path; *cursor != '\0'; ++cursor )
    {
        if( *cursor == '/' || *cursor == '\\' )
        {
            result = cursor + 1;
        }
    }

    return result;
}
//////////////////////////////////////////////////////////////////////////
const char * moviec_path_extension( const char * _path )
{
    const char * name = moviec_path_filename( _path );
    const char * result = strrchr( name, '.' );

    const char * extension = result != NULL ? result : "";

    return extension;
}
//////////////////////////////////////////////////////////////////////////
char * moviec_path_stem( MoviecContext * _context, const char * _path )
{
    const char * name = moviec_path_filename( _path );
    const char * extension = strrchr( name, '.' );
    size_t length = extension != NULL ? (size_t)( extension - name ) : strlen( name );
    char * result = moviec_string_copy( _context, name, length );

    return result;
}
//////////////////////////////////////////////////////////////////////////
char * moviec_path_directory( MoviecContext * _context, const char * _path )
{
    const char * name = moviec_path_filename( _path );
    size_t length = (size_t)( name - _path );

    if( length == 0 )
    {
        char * result = moviec_string_copy( _context, ".", 1 );

        return result;
    }

    while( length > 1 && ( _path[length - 1] == '/' || _path[length - 1] == '\\' ) )
    {
        --length;
    }

    char * result = moviec_string_copy( _context, _path, length );

    return result;
}
//////////////////////////////////////////////////////////////////////////
char * moviec_path_join( MoviecContext * _context, const char * _base, const char * _path )
{
    if( _path[0] == '/' || _path[0] == '\\' || ( isalpha( (unsigned char)_path[0] ) != 0 && _path[1] == ':' ) )
    {
        char * result = moviec_format( _context, "%s", _path );

        return result;
    }

    char * result = moviec_format( _context, "%s/%s", _base, _path );

    return result;
}
//////////////////////////////////////////////////////////////////////////
char * moviec_path_absolute( MoviecContext * _context, const char * _path )
{
    char current[4096];
    moviec_require( _context, MOVIEC_GETCWD( current, sizeof( current ) ) != NULL, "cannot get current directory" );
    char * joined = moviec_path_join( _context, current, _path );

    for( char * cursor = joined; *cursor != '\0'; ++cursor )
    {
        if( *cursor == '\\' )
        {
            *cursor = '/';
        }
    }

    size_t length = strlen( joined );
    char * result = (char *)moviec_allocate( _context, length + 2 );
    size_t * offsets = (size_t *)moviec_allocate( _context, ( length + 1 ) * sizeof( size_t ) );
    size_t depth = 0;
    size_t used = 0;
    const char * cursor = joined;

    if( cursor[0] == '/' )
    {
        result[used++] = '/';
        ++cursor;
    }

    while( *cursor != '\0' )
    {
        const char * end = strchr( cursor, '/' );
        size_t size = end != NULL ? (size_t)( end - cursor ) : strlen( cursor );

        if( size != 0 && !( size == 1 && cursor[0] == '.' ) )
        {
            if( size == 2 && cursor[0] == '.' && cursor[1] == '.' )
            {
                if( depth != 0 )
                {
                    used = offsets[--depth];
                }
            }
            else
            {
                offsets[depth++] = used;

                if( used != 0 && result[used - 1] != '/' )
                {
                    result[used++] = '/';
                }

                memcpy( result + used, cursor, size );
                used += size;
            }
        }

        if( end == NULL )
        {
            break;
        }

        cursor = end + 1;
    }

    result[used] = '\0';

    return result;
}
//////////////////////////////////////////////////////////////////////////
char * moviec_path_relative( MoviecContext * _context, const char * _path, const char * _base )
{
    char * path = moviec_path_absolute( _context, _path );
    char * base = moviec_path_absolute( _context, _base );
    size_t common = 0;
    size_t boundary = 0;

    while( path[common] != '\0' && path[common] == base[common] )
    {
        if( path[common] == '/' )
        {
            boundary = common + 1;
        }

        ++common;
    }

    if( base[common] == '\0' && ( path[common] == '/' || path[common] == '\0' ) )
    {
        boundary = common + ( path[common] == '/' ? 1 : 0 );
        base += common;
    }
    else
    {
        base += boundary;
    }

    MoviecWriter result = { 0 };

    for( const char * cursor = base; *cursor != '\0'; ++cursor )
    {
        if( cursor == base || *cursor == '/' )
        {
            moviec_write_raw( _context, &result, "../", 3 );
        }
    }

    moviec_write_raw( _context, &result, path + boundary, strlen( path + boundary ) + 1 );

    char * relative = (char *)result.bytes;

    return relative;
}
//////////////////////////////////////////////////////////////////////////
int moviec_file_exists( const char * _path )
{
    struct stat info;

    if( stat( _path, &info ) != 0 )
    {
        return 0;
    }

    int result = ( info.st_mode & S_IFMT ) == S_IFREG;

    return result;
}
//////////////////////////////////////////////////////////////////////////
void moviec_create_directories( MoviecContext * _context, const char * _path )
{
    if( _path[0] == '\0' )
    {
        return;
    }

    char * copy = moviec_format( _context, "%s", _path );

    for( char * cursor = copy + 1;; ++cursor )
    {
        if( *cursor != '/' && *cursor != '\\' && *cursor != '\0' )
        {
            continue;
        }

        char saved = *cursor;
        *cursor = '\0';
        int result = MOVIEC_MKDIR( copy );
        int error = errno;
        *cursor = saved;
        moviec_require( _context, result == 0 || error == EEXIST, "cannot create directory: %s", copy );

        if( saved == '\0' )
        {
            break;
        }
    }
}
//////////////////////////////////////////////////////////////////////////
MoviecWriter moviec_read_file( MoviecContext * _context, const char * _path )
{
    struct stat info;
    moviec_require( _context, stat( _path, &info ) == 0, "cannot read file: %s", _path );
    moviec_require( _context, info.st_size >= 0 && info.st_size <= 67108864, "file is too large" );
    MoviecWriter result = { 0 };
    result.size = (size_t)info.st_size;
    result.bytes = (uint8_t *)moviec_allocate( _context, result.size + 1 );
    FILE * file = fopen( _path, "rb" );
    moviec_require( _context, file != NULL, "cannot open file: %s", _path );
    size_t read = fread( result.bytes, 1, result.size, file );
    int closed = fclose( file );
    moviec_require( _context, read == result.size && closed == 0, "cannot read file: %s", _path );

    return result;
}
//////////////////////////////////////////////////////////////////////////
void moviec_write_file( MoviecContext * _context, const char * _path, const void * _bytes, size_t _size )
{
    const char * directory = moviec_path_directory( _context, _path );
    moviec_create_directories( _context, directory );
    FILE * file = fopen( _path, "wb" );
    moviec_require( _context, file != NULL, "cannot write file: %s", _path );
    size_t written = fwrite( _bytes, 1, _size, file );
    int closed = fclose( file );
    moviec_require( _context, written == _size && closed == 0, "cannot write file: %s", _path );
}
//////////////////////////////////////////////////////////////////////////
void moviec_save_json( MoviecContext * _context, const char * _path, json_t * _value )
{
    const char * directory = moviec_path_directory( _context, _path );
    moviec_create_directories( _context, directory );
    int result = json_dump_file( _value, _path, JSON_INDENT( 2 ) | JSON_SORT_KEYS | JSON_ENCODE_ANY );
    moviec_require( _context, result == 0, "cannot write JSON: %s", _path );
}
//////////////////////////////////////////////////////////////////////////
void moviec_write_raw( MoviecContext * _context, MoviecWriter * _writer, const void * _bytes, size_t _size )
{
    moviec_require( _context, _size <= 67108864U - _writer->size, "binary is too large" );
    size_t needed = _writer->size + _size;

    if( needed > _writer->capacity )
    {
        size_t capacity = _writer->capacity == 0 ? 256 : _writer->capacity;

        while( capacity < needed )
        {
            capacity *= 2;
        }

        _writer->bytes = (uint8_t *)moviec_grow( _context, _writer->bytes, _writer->size, capacity );
        _writer->capacity = capacity;
    }

    if( _size != 0 )
    {
        memcpy( _writer->bytes + _writer->size, _bytes, _size );
    }

    _writer->size = needed;
}
//////////////////////////////////////////////////////////////////////////
void moviec_write_byte( MoviecContext * _context, MoviecWriter * _writer, uint32_t _value )
{
    moviec_require( _context, _value <= 255, "byte overflow" );
    uint8_t value = (uint8_t)_value;
    moviec_write_raw( _context, _writer, &value, 1 );
}
//////////////////////////////////////////////////////////////////////////
void moviec_write_u32( MoviecContext * _context, MoviecWriter * _writer, uint32_t _value )
{
    uint8_t bytes[4];

    for( unsigned index = 0; index != 4; ++index )
    {
        bytes[index] = (uint8_t)( _value >> ( index * 8 ) );
    }

    moviec_write_raw( _context, _writer, bytes, 4 );
}
//////////////////////////////////////////////////////////////////////////
void moviec_write_float( MoviecContext * _context, MoviecWriter * _writer, float _value )
{
    uint32_t bits;
    memcpy( &bits, &_value, 4 );
    moviec_write_u32( _context, _writer, bits );
}
//////////////////////////////////////////////////////////////////////////
void moviec_write_string( MoviecContext * _context, MoviecWriter * _writer, const char * _value )
{
    size_t length = strlen( _value );
    moviec_require( _context, length <= 4096, "string too long" );
    moviec_write_u32( _context, _writer, (uint32_t)length );
    moviec_write_raw( _context, _writer, _value, length );
}
//////////////////////////////////////////////////////////////////////////
void moviec_write_size( MoviecContext * _context, MoviecWriter * _writer, size_t _value )
{
    moviec_require( _context, _value <= 1048576, "count too large" );

    if( _value < 255 )
    {
        moviec_write_byte( _context, _writer, (uint32_t)_value );

        return;
    }

    moviec_write_byte( _context, _writer, 255 );

    if( _value < 65535 )
    {
        moviec_write_byte( _context, _writer, (uint32_t)_value & 255 );
        moviec_write_byte( _context, _writer, (uint32_t)_value >> 8 );

        return;
    }

    moviec_write_byte( _context, _writer, 255 );
    moviec_write_byte( _context, _writer, 255 );
    moviec_write_u32( _context, _writer, (uint32_t)_value );
}
//////////////////////////////////////////////////////////////////////////
void moviec_write_legacy_string( MoviecContext * _context, MoviecWriter * _writer, const char * _value )
{
    size_t length = strlen( _value );
    moviec_write_size( _context, _writer, length );
    moviec_write_raw( _context, _writer, _value, length );
}
//////////////////////////////////////////////////////////////////////////
uint32_t moviec_read_u32( const uint8_t * _bytes )
{
    uint32_t result = (uint32_t)_bytes[0] | ( (uint32_t)_bytes[1] << 8 ) | ( (uint32_t)_bytes[2] << 16 ) | ( (uint32_t)_bytes[3] << 24 );

    return result;
}
