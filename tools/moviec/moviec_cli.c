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
#include <stdlib.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
typedef struct MoviecOptions
{
    const char * command;
    const char * output;
    const char * composition;
    const char * packageRoot;
    const char * resourcePrefix;
    const char * key;
    json_t * parameters;
    float time;
} MoviecOptions;
//////////////////////////////////////////////////////////////////////////
static void * moviec_sdk_allocate( void * _userdata, size_t _size )
{
    (void)_userdata;
    void * memory = malloc( _size );

    return memory;
}
//////////////////////////////////////////////////////////////////////////
static void * moviec_sdk_allocate_many( void * _userdata, size_t _size, size_t _count )
{
    (void)_userdata;
    void * memory = calloc( _count, _size );

    return memory;
}
//////////////////////////////////////////////////////////////////////////
static void moviec_sdk_free( void * _userdata, const void * _memory )
{
    (void)_userdata;
    free( (void *)_memory );
}
//////////////////////////////////////////////////////////////////////////
static size_t moviec_sdk_read( void * _output, size_t _position, size_t _size, void * _userdata )
{
    const MoviecWriter * input = _userdata;

    if( _position > input->size || _size > input->size - _position )
    {
        memset( _output, 0, _size );

        return 0;
    }

    memcpy( _output, input->bytes + _position, _size );

    return _size;
}
//////////////////////////////////////////////////////////////////////////
static void moviec_sdk_copy( const void * _source, void * _destination, size_t _size, void * _userdata )
{
    (void)_userdata;
    memcpy( _destination, _source, _size );
}
//////////////////////////////////////////////////////////////////////////
void moviec_load( MoviecContext * _context, const MoviecWriter * _input, const char * _key )
{
    moviec_require( _context, _input->size >= 16, "binary is truncated" );
    _context->instance = ae_create_movie_instance( _key[0] == '\0' ? AE_HASHKEY_EMPTY : _key, moviec_sdk_allocate, moviec_sdk_allocate_many, moviec_sdk_free, moviec_sdk_free, NULL, NULL, NULL );
    moviec_require( _context, _context->instance != NULL, "cannot create movie instance" );
    aeMovieDataProviders providers;
    ae_clear_movie_data_providers( &providers );
    _context->movieData = ae_create_movie_data( _context->instance, &providers, NULL );
    moviec_require( _context, _context->movieData != NULL, "cannot create movie data" );
    aeMovieStream * stream = ae_create_movie_stream( _context->instance, moviec_sdk_read, moviec_sdk_copy, (void *)_input );
    moviec_require( _context, stream != NULL, "cannot create movie stream" );
    ae_result_t result = ae_load_movie_data( _context->movieData, stream, &_context->major, &_context->minor );
    ae_delete_movie_stream( stream );
    moviec_require( _context, result == AE_RESULT_SUCCESSFUL, "%s", ae_get_movie_result_string_info( result ) );
}
//////////////////////////////////////////////////////////////////////////
static float moviec_option_number( MoviecContext * _context, const char * _text )
{
    errno = 0;
    char * end;
    float value = strtof( _text, &end );
    moviec_require( _context, end != _text && *end == '\0' && errno != ERANGE && isfinite( value ) != 0, "invalid numeric option: %s", _text );

    return value;
}
//////////////////////////////////////////////////////////////////////////
static MoviecOptions moviec_options( MoviecContext * _context, int _argc, char * _argv[] )
{
    moviec_require( _context, _argc >= 3, "usage: moviec validate|build|inspect|sample input [-o output.aem] [--composition id] [--time seconds] [--param name=value]" );
    _context->source = _argv[2];
    MoviecOptions options = { _argv[1], "", "", "", "", "", NULL, 0.f };
    options.parameters = moviec_object( _context );

    for( int index = 3; index < _argc; ++index )
    {
        const char * argument = _argv[index];
        moviec_require( _context, index + 1 < _argc, "option requires a value: %s", argument );
        const char * value = _argv[++index];

        if( strcmp( argument, "-o" ) == 0 )
        {
            options.output = value;
        }
        else if( strcmp( argument, "--composition" ) == 0 )
        {
            options.composition = value;
        }
        else if( strcmp( argument, "--time" ) == 0 )
        {
            options.time = moviec_option_number( _context, value );
        }
        else if( strcmp( argument, "--asset-root" ) == 0 )
        {
            _context->assetRoot = moviec_path_absolute( _context, value );
        }
        else if( strcmp( argument, "--package-root" ) == 0 )
        {
            options.packageRoot = moviec_path_absolute( _context, value );
        }
        else if( strcmp( argument, "--resource-prefix" ) == 0 )
        {
            options.resourcePrefix = value;
        }
        else if( strcmp( argument, "--key-file" ) == 0 )
        {
            json_error_t error;
            json_t * config = moviec_own_json( _context, json_load_file( value, JSON_REJECT_DUPLICATES, &error ) );
            options.key = moviec_text( moviec_get( moviec_get( config, "MoviePlugin" ), "HASHKEY" ), "" );
            moviec_require( _context, strlen( options.key ) == 40, "MoviePlugin.HASHKEY must contain 40 lowercase hexadecimal digits" );

            for( const char * digit = options.key; *digit != '\0'; ++digit )
            {
                moviec_require( _context, ( *digit >= '0' && *digit <= '9' ) || ( *digit >= 'a' && *digit <= 'f' ), "MoviePlugin.HASHKEY must contain 40 lowercase hexadecimal digits" );
            }
        }
        else if( strcmp( argument, "--param" ) == 0 )
        {
            const char * equal = strchr( value, '=' );
            moviec_require( _context, equal != NULL && equal != value, "--param expects name=value" );
            const char * name = moviec_string_copy( _context, value, (size_t)( equal - value ) );
            moviec_json_number( _context, options.parameters, name, moviec_option_number( _context, equal + 1 ) );
        }
        else
        {
            moviec_fail( _context, "unknown option: %s", argument );
        }
    }

    return options;
}
//////////////////////////////////////////////////////////////////////////
static void moviec_prefix_resources( MoviecContext * _context, const char * _prefix )
{
    if( _prefix[0] == '\0' )
    {
        return;
    }

    json_t * assets = moviec_get( _context->document, "resources" );
    json_t * renamed = moviec_object( _context );
    json_t * names = moviec_keys( _context, assets );

    for( size_t index = 0; index != json_array_size( names ); ++index )
    {
        const char * id = json_string_value( json_array_get( names, index ) );
        json_t * asset = moviec_get( assets, id );
        json_t * images = moviec_get( asset, "images" );

        for( size_t imageIndex = 0; imageIndex != json_array_size( images ); ++imageIndex )
        {
            const char * image = moviec_text( json_array_get( images, imageIndex ), "" );
            const char * prefixed = moviec_format( _context, "%s%s", _prefix, image );
            moviec_require( _context, json_array_set_new( images, imageIndex, json_string( prefixed ) ) == 0, "cannot prefix image" );
        }

        moviec_json_set( _context, renamed, moviec_format( _context, "%s%s", _prefix, id ), asset );
    }

    moviec_json_set( _context, _context->document, "resources", renamed );
    const char * name = moviec_text( moviec_get( _context->document, "name" ), "movie" );
    moviec_json_string( _context, _context->document, "name", moviec_format( _context, "%s%s", _prefix, name ) );
    json_t * list = moviec_get( _context->document, "compositions" );

    for( size_t index = 0; index != json_array_size( list ); ++index )
    {
        json_t * layers = moviec_get( json_array_get( list, index ), "layers" );

        for( size_t layerIndex = 0; layerIndex != json_array_size( layers ); ++layerIndex )
        {
            json_t * layer = json_array_get( layers, layerIndex );
            json_t * resource = moviec_get( layer, "resource" );

            if( resource == NULL )
            {
                continue;
            }

            const char * asset = moviec_text( resource, "" );
            moviec_json_string( _context, layer, "resource", moviec_format( _context, "%s%s", _prefix, asset ) );
        }
    }
}
//////////////////////////////////////////////////////////////////////////
static void moviec_bind_key( MoviecWriter * _bytes, const char * _key )
{
    uint32_t hash = 0;

    for( size_t index = 0; _key[index] != '\0'; ++index )
    {
        uint32_t value = (uint32_t)( _key[index] > '9' ? _key[index] - 'a' + 10 : _key[index] - '0' );
        hash ^= value << ( ( index % 8 ) * 4 );
    }

    for( unsigned index = 0; index != 4; ++index )
    {
        _bytes->bytes[12 + index] = (uint8_t)( hash >> ( index * 8 ) );
    }
}
//////////////////////////////////////////////////////////////////////////
static void moviec_build_output( MoviecContext * _context, const MoviecOptions * _options, MoviecWriter _bytes, const char * _compiler )
{
    moviec_require( _context, _options->output[0] != '\0', "build requires -o output.aem" );
    MoviecWriter deployed = _bytes;

    if( strcmp( moviec_path_extension( _options->output ), ".aez" ) == 0 )
    {
        deployed = moviec_pack( _context, _bytes );
    }

    const char * temporary = moviec_format( _context, "%s.tmp", _options->output );
    moviec_write_file( _context, temporary, deployed.bytes, deployed.size );
    moviec_require( _context, rename( temporary, _options->output ) == 0, "cannot replace binary: %s", _options->output );
    moviec_save_json( _context, moviec_format( _context, "%s.resources.json", _options->output ), _context->manifest );
    moviec_save_json( _context, moviec_format( _context, "%s.map.json", _options->output ), _context->sourceMap );
    moviec_write_resources( _context, _options->output, _options->packageRoot, _options->resourcePrefix[0] != '\0' );
    json_t * authoring = moviec_object( _context );
    moviec_json_string( _context, authoring, "source", moviec_path_absolute( _context, _context->source ) );
    moviec_json_string( _context, authoring, "compiler", moviec_path_absolute( _context, _compiler ) );
    moviec_json_string( _context, authoring, "assetRoot", _context->assetRoot );
    moviec_json_string( _context, authoring, "packageRoot", _options->packageRoot );
    moviec_json_string( _context, authoring, "output", moviec_path_absolute( _context, _options->output ) );
    moviec_save_json( _context, moviec_format( _context, "%s.authoring.json", _options->output ), authoring );
}
//////////////////////////////////////////////////////////////////////////
static void moviec_sample( MoviecContext * _context, const MoviecOptions * _options, json_t * _report )
{
    const aeMovieCompositionData * data = _options->composition[0] == '\0' ? ae_get_movie_composition_data_by_index( _context->movieData, 0 ) : ae_get_movie_composition_data( _context->movieData, _options->composition );
    moviec_require( _context, data != NULL, "composition not found" );
    aeMovieCompositionProviders providers;
    ae_initialize_movie_composition_providers( &providers );
    _context->composition = ae_create_movie_composition( data, AE_TRUE, &providers, NULL );
    moviec_require( _context, _context->composition != NULL, "cannot create composition" );
    const char * name;
    json_t * parameter;
    json_object_foreach( _options->parameters, name, parameter )
    {
        float value = moviec_number( _context, parameter, 0.f );
        ae_bool_t applied = AE_FALSE;

        for( uint32_t index = 0; index != AE_MOVIE_MOTION_PARAMETER_COUNT; ++index )
        {
            aeMovieMotionParameter standard = (aeMovieMotionParameter)index;

            if( strcmp( name, ae_get_movie_motion_parameter_name( standard ) ) != 0 )
            {
                continue;
            }

            applied = ae_set_movie_motion_parameter( _context->composition, standard, value );
            break;
        }

        if( applied == AE_FALSE )
        {
            uint32_t index;

            if( ae_find_movie_motion_register( _context->movieData, name, &index ) == AE_TRUE )
            {
                applied = ae_set_movie_motion_register( _context->composition, index, value );
            }
        }

        moviec_require( _context, applied == AE_TRUE, "unknown or invalid parameter: %s", name );
    }

    json_t * values = moviec_object( _context );

    for( uint32_t index = 0; index != ae_get_movie_motion_channel_count( _context->movieData ); ++index )
    {
        const char * channel = ae_get_movie_motion_channel_name( _context->movieData, index );
        float value;

        if( ae_evaluate_movie_motion_channel( _context->composition, channel, _options->time, &value ) == AE_FALSE )
        {
            continue;
        }

        moviec_json_number( _context, values, channel, value );
    }

    moviec_json_set( _context, _report, "values", values );
}
//////////////////////////////////////////////////////////////////////////
typedef struct MoviecLayerReport
{
    MoviecContext * context;
    json_t * layers;
} MoviecLayerReport;
//////////////////////////////////////////////////////////////////////////
static ae_bool_t moviec_report_layer( const aeMovieCompositionData * _composition, const aeMovieLayerData * _layer, void * _userdata )
{
    (void)_composition;
    MoviecLayerReport * report = _userdata;
    const aeMovieResource * resource = ae_get_movie_layer_data_resource( _layer );
    json_t * item = moviec_object( report->context );
    moviec_json_string( report->context, item, "id", ae_get_movie_layer_data_name( _layer ) );
    moviec_json_string( report->context, item, "resource", resource != NULL ? resource->name : "" );
    moviec_json_append( report->context, report->layers, item );

    return AE_TRUE;
}
//////////////////////////////////////////////////////////////////////////
static json_t * moviec_report( MoviecContext * _context, const MoviecOptions * _options, size_t _size )
{
    json_t * report = moviec_object( _context );
    moviec_json_string( _context, report, "format", "AEM1" );
    moviec_json_integer( _context, report, "major", _context->major );
    moviec_json_integer( _context, report, "minor", _context->minor );
    moviec_json_integer( _context, report, "channels", ae_get_movie_motion_channel_count( _context->movieData ) );
    moviec_json_integer( _context, report, "bytes", (json_int_t)_size );
    json_t * parameters = moviec_object( _context );
    json_t * registers = moviec_object( _context );

    if( ae_is_movie_motion_data( _context->movieData ) == AE_TRUE )
    {
        for( uint32_t index = 0; index != AE_MOVIE_MOTION_PARAMETER_COUNT; ++index )
        {
            const char * name = ae_get_movie_motion_parameter_name( (aeMovieMotionParameter)index );
            moviec_json_integer( _context, parameters, name, index );
        }

        for( uint32_t index = 0; index != ae_get_movie_motion_register_count( _context->movieData ); ++index )
        {
            const char * name = ae_get_movie_motion_register_name( _context->movieData, index );
            moviec_json_integer( _context, registers, name, index );
        }
    }

    moviec_json_set( _context, report, "parameters", parameters );
    moviec_json_set( _context, report, "registers", registers );

    if( strcmp( _options->command, "sample" ) == 0 )
    {
        moviec_sample( _context, _options, report );

        return report;
    }

    json_t * list = moviec_array( _context );

    for( uint32_t index = 0; index != ae_get_movie_composition_data_count( _context->movieData ); ++index )
    {
        const aeMovieCompositionData * data = ae_get_movie_composition_data_by_index( _context->movieData, index );
        const char * name = ae_get_movie_composition_data_name( data );

        if( _options->composition[0] != '\0' && strcmp( _options->composition, name ) != 0 )
        {
            continue;
        }

        json_t * item = moviec_object( _context );
        moviec_json_string( _context, item, "id", name );
        moviec_json_number( _context, item, "duration", ae_get_movie_composition_data_duration( data ) );
        MoviecLayerReport layerReport = { _context, moviec_array( _context ) };
        ae_visit_composition_layer_data( data, moviec_report_layer, &layerReport );
        moviec_json_set( _context, item, "layers", layerReport.layers );
        moviec_json_append( _context, list, item );
    }

    moviec_json_set( _context, report, "compositions", list );

    return report;
}
//////////////////////////////////////////////////////////////////////////
static void moviec_execute( MoviecContext * _context, int _argc, char * _argv[], FILE * _output )
{
    MoviecOptions options = moviec_options( _context, _argc, _argv );
    MoviecWriter bytes;

    if( strcmp( options.command, "build" ) == 0 || strcmp( options.command, "validate" ) == 0 )
    {
        json_error_t error;
        json_t * document = json_load_file( _context->source, JSON_REJECT_DUPLICATES, &error );
        moviec_require( _context, document != NULL, "JSON line %d: %s", error.line, error.text );
        _context->document = moviec_own_json( _context, document );

        if( _context->assetRoot == NULL )
        {
            _context->assetRoot = moviec_path_directory( _context, moviec_path_absolute( _context, _context->source ) );
        }

        moviec_prefix_resources( _context, options.resourcePrefix );
        bytes = moviec_compile( _context );
        moviec_bind_key( &bytes, options.key );
    }
    else
    {
        moviec_require( _context, strcmp( options.command, "inspect" ) == 0 || strcmp( options.command, "sample" ) == 0, "unknown command: %s", options.command );
        bytes = moviec_unpack( _context, moviec_read_file( _context, _context->source ) );
    }

    moviec_load( _context, &bytes, options.key );

    if( strcmp( options.command, "build" ) == 0 )
    {
        moviec_build_output( _context, &options, bytes, _argv[0] );
    }

    json_t * report = moviec_report( _context, &options, bytes.size );
    moviec_require( _context, json_dumpf( report, _output, JSON_COMPACT | JSON_SORT_KEYS ) == 0 && fputc( '\n', _output ) != EOF, "cannot write report" );
}
//////////////////////////////////////////////////////////////////////////
int moviec_run( int _argc, char * _argv[], FILE * _output, FILE * _error )
{
    MoviecContext * context = calloc( 1, sizeof( MoviecContext ) );

    if( context == NULL )
    {
        fputs( "{\"error\":\"out of memory\"}\n", _error );

        return 1;
    }

    context->path = "/";
    context->source = "";
    context->ownedJson = json_array();
    int result = 0;

    if( setjmp( context->errorJump ) == 0 )
    {
        moviec_require( context, context->ownedJson != NULL, "out of memory" );
        moviec_execute( context, _argc, _argv, _output );
    }
    else
    {
        json_t * report = json_pack( "{s:s,s:s,s:s}", "file", context->source, "path", context->path, "error", context->error );

        if( report != NULL )
        {
            json_dumpf( report, _error, JSON_COMPACT | JSON_SORT_KEYS );
            json_decref( report );
        }
        else
        {
            fputs( "{\"error\":\"out of memory\"}", _error );
        }

        fputc( '\n', _error );
        result = 1;
    }

    moviec_finalize( context );
    free( context );

    return result;
}
