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

#include "moviec.h"

#include <jansson.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////
static const char * TEST_DIRECTORY;
//////////////////////////////////////////////////////////////////////////
static void check( int _condition, const char * _message )
{
    if( _condition != 0 )
    {
        return;
    }

    fprintf( stderr, "moviec check failed: %s\n", _message );
    exit( EXIT_FAILURE );
}
//////////////////////////////////////////////////////////////////////////
static json_t * parse( const char * _source )
{
    json_error_t error;
    json_t * result = json_loads( _source, JSON_REJECT_DUPLICATES, &error );

    if( result == NULL )
    {
        fprintf( stderr, "%s\n", error.text );
    }

    check( result != NULL, "invalid test fixture" );

    return result;
}
//////////////////////////////////////////////////////////////////////////
static void path( char * _output, size_t _capacity, const char * _name )
{
    int size = snprintf( _output, _capacity, "%s/%s", TEST_DIRECTORY, _name );
    check( size > 0 && (size_t)size < _capacity, "test path overflow" );
}
//////////////////////////////////////////////////////////////////////////
static json_t * run( const char * _command, const char * _input, const char * const * _options, size_t _optionCount, int _success )
{
    char * arguments[16] = { "moviec", (char *)_command, (char *)_input };
    check( _optionCount <= 13, "too many test options" );

    for( size_t index = 0; index != _optionCount; ++index )
    {
        arguments[index + 3] = (char *)_options[index];
    }

    FILE * output = tmpfile();
    FILE * error = tmpfile();
    check( output != NULL && error != NULL, "cannot capture CLI output" );
    int result = moviec_run( (int)_optionCount + 3, arguments, output, error );
    FILE * selected = result == 0 ? output : error;
    rewind( selected );
    json_error_t jsonError;
    json_t * report = json_loadf( selected, 0, &jsonError );
    fclose( output );
    fclose( error );

    if( ( result == 0 ) != ( _success != 0 ) && report != NULL )
    {
        json_dumpf( report, stderr, JSON_COMPACT );
        fputc( '\n', stderr );
    }

    check( ( result == 0 ) == ( _success != 0 ), "unexpected CLI status" );
    check( report != NULL, "CLI response must be JSON" );

    if( _success == 0 )
    {
        const char * message = json_string_value( json_object_get( report, "error" ) );
        const char * jsonPath = json_string_value( json_object_get( report, "path" ) );
        check( message != NULL && message[0] != '\0' && jsonPath != NULL && jsonPath[0] == '/', "incomplete diagnostic" );
    }

    return report;
}
//////////////////////////////////////////////////////////////////////////
static json_t * base_document( void )
{
    json_t * result = parse( "{\"version\":1,\"name\":\"Checks\",\"parameters\":{\"gain\":2},\"resources\":{},\"compositions\":[{\"id\":\"Main\",\"size\":[100,100],\"duration\":4,\"layers\":[{\"id\":\"control\",\"channels\":{}}]}]}" );

    return result;
}
//////////////////////////////////////////////////////////////////////////
static json_t * first_layer( json_t * _document )
{
    json_t * composition = json_array_get( json_object_get( _document, "compositions" ), 0 );
    json_t * result = json_array_get( json_object_get( composition, "layers" ), 0 );

    return result;
}
//////////////////////////////////////////////////////////////////////////
static void set_json( json_t * _object, const char * _name, const char * _source )
{
    check( json_object_set_new( _object, _name, parse( _source ) ) == 0, "cannot set fixture field" );
}
//////////////////////////////////////////////////////////////////////////
static json_t * compile( json_t * _document, const char * _output, int _success )
{
    char source[4096];
    path( source, sizeof( source ), "check.movie" );
    check( json_dump_file( _document, source, JSON_SORT_KEYS ) == 0, "cannot write fixture" );
    const char * options[] = { "-o", _output };
    json_t * result = run( "build", source, options, 2, _success );

    return result;
}
//////////////////////////////////////////////////////////////////////////
static double channel( json_t * _report, const char * _name )
{
    json_t * value = json_object_get( json_object_get( _report, "values" ), _name );
    check( json_is_number( value ), _name );
    double result = json_number_value( value );

    return result;
}
//////////////////////////////////////////////////////////////////////////
static unsigned char * read_bytes( const char * _path, size_t * _size )
{
    FILE * file = fopen( _path, "rb" );
    check( file != NULL, "cannot open binary fixture" );
    check( fseek( file, 0, SEEK_END ) == 0, "cannot seek binary fixture" );
    long size = ftell( file );
    check( size >= 0, "cannot measure binary fixture" );
    rewind( file );
    unsigned char * result = malloc( (size_t)size );
    check( result != NULL, "out of memory" );
    check( fread( result, 1, (size_t)size, file ) == (size_t)size, "cannot read binary fixture" );
    check( fclose( file ) == 0, "cannot close binary fixture" );
    *_size = (size_t)size;

    return result;
}
//////////////////////////////////////////////////////////////////////////
static void test_channels( const char * _binary )
{
    json_t * document = base_document();
    set_json( first_layer( document ), "channels",
              "{\"custom.hold\":{\"keys\":[{\"time\":0,\"value\":1,\"interpolation\":\"hold\"},{\"time\":2,\"value\":5}]},"
              "\"custom.cycle\":{\"extrapolation\":\"cycle\",\"keys\":[{\"time\":0,\"value\":0},{\"time\":1,\"value\":10}]},"
              "\"custom.ping\":{\"extrapolation\":\"pingpong\",\"keys\":[{\"time\":0,\"value\":0},{\"time\":1,\"value\":10}]},"
              "\"custom.baked\":{\"samples\":[0,10,20],\"sampleRate\":1},"
              "\"custom.constants\":{\"expression\":\"sin(pi / 2) + cos(tau)\"},"
              "\"custom.controls\":{\"expression\":\"params.speed + params.intensity + params.amplitude + params.frequency + params.phase + params.progress + params.blend + params.directionX + params.directionY + params.targetX + params.targetY + params.damping + params.seed\"},"
              "\"custom.math\":{\"expression\":\"time > 1 ? clamp(pow(params.gain, 2), 0, 10) : 0\"},"
              "\"custom.deterministic\":{\"expression\":\"wiggle(2,5,time,12) + noise(time,9) + spring(1,2,3,time)\"},"
              "\"custom.invalidMath\":{\"value\":7,\"expression\":\"sqrt(-1)\"},"
              "\"position\":{\"value\":[1,2],\"expression\":\"value + [params.gain, 3]\"}}" );
    json_t * report = compile( document, _binary, 1 );
    check( strcmp( json_string_value( json_object_get( report, "format" ) ), "AEM1" ) == 0, "file signature" );
    check( json_integer_value( json_object_get( report, "major" ) ) == 18 && json_integer_value( json_object_get( report, "minor" ) ) == 0, "SDK version" );
    check( json_integer_value( json_object_get( json_object_get( report, "registers" ), "gain" ) ) == 0, "legacy register index" );
    check( json_integer_value( json_object_get( json_object_get( report, "parameters" ), "intensity" ) ) == 1, "standard parameter index" );
    json_decref( report );
    size_t originalSize;
    unsigned char * original = read_bytes( _binary, &originalSize );
    json_decref( compile( document, _binary, 1 ) );
    size_t rebuiltSize;
    unsigned char * rebuilt = read_bytes( _binary, &rebuiltSize );
    check( rebuiltSize == originalSize && memcmp( rebuilt, original, rebuiltSize ) == 0, "deterministic binary" );
    free( original );
    free( rebuilt );
    const char * options[] = { "--time", "1.25" };
    report = run( "sample", _binary, options, 2, 1 );
    check( fabs( channel( report, "Main.control.custom.constants" ) - 2.0 ) < 1.e-5, "constants" );
    check( channel( report, "Main.control.custom.hold" ) == 1.0, "hold keys" );
    check( fabs( channel( report, "Main.control.custom.cycle" ) - 2.5 ) < 1.e-5, "cycle extrapolation" );
    check( fabs( channel( report, "Main.control.custom.ping" ) - 7.5 ) < 1.e-5, "pingpong extrapolation" );
    check( fabs( channel( report, "Main.control.custom.baked" ) - 12.5 ) < 1.e-5, "baked samples" );
    check( channel( report, "Main.control.custom.math" ) == 4.0, "expressions" );
    check( channel( report, "Main.control.custom.controls" ) == 6.0, "standard defaults" );
    check( channel( report, "Main.control.custom.invalidMath" ) == 7.0, "invalid expression fallback" );
    check( channel( report, "Main.control.position.x" ) == 3.0 && channel( report, "Main.control.position.y" ) == 5.0, "vector expressions" );
    json_t * repeated = run( "sample", _binary, options, 2, 1 );
    check( json_equal( json_object_get( report, "values" ), json_object_get( repeated, "values" ) ), "deterministic sampling" );
    json_decref( repeated );
    const char * intensity[] = { "--time", "1.25", "--param", "intensity=0.5" };
    repeated = run( "sample", _binary, intensity, 4, 1 );
    check( channel( repeated, "Main.control.custom.controls" ) == 5.5, "standard override" );
    json_decref( repeated );
    const char * gain[] = { "--time", "1.25", "--param", "gain=3" };
    repeated = run( "sample", _binary, gain, 4, 1 );
    check( channel( repeated, "Main.control.custom.math" ) == 9.0, "register override" );
    json_decref( repeated );
    char archive[4096];
    path( archive, sizeof( archive ), "check.aez" );
    json_decref( compile( document, archive, 1 ) );
    repeated = run( "sample", archive, options, 2, 1 );
    check( json_equal( json_object_get( report, "values" ), json_object_get( repeated, "values" ) ), "AEZ equivalence" );
    json_decref( repeated );
    json_decref( report );
    json_decref( document );
}
//////////////////////////////////////////////////////////////////////////
static void test_registers( const char * _binary )
{
    json_t * document = base_document();
    set_json( document, "registers", "[{\"name\":\"zeta\",\"value\":3},{\"name\":\"alpha\",\"value\":4}]" );
    set_json( first_layer( document ), "channels", "{\"custom.registers\":{\"expression\":\"params.zeta + params.alpha\"}}" );
    json_t * report = compile( document, _binary, 1 );
    json_t * expected = parse( "{\"zeta\":0,\"alpha\":1,\"gain\":2}" );
    check( json_equal( json_object_get( report, "registers" ), expected ), "ordered registers" );
    json_decref( expected );
    json_decref( report );
    report = run( "sample", _binary, NULL, 0, 1 );
    check( channel( report, "Main.control.custom.registers" ) == 7.0, "register evaluation" );
    json_decref( report );
    json_t * registers = json_object_get( document, "registers" );
    check( json_array_append_new( registers, parse( "{\"name\":\"last\",\"value\":5}" ) ) == 0, "append register" );
    report = compile( document, _binary, 1 );
    expected = parse( "{\"zeta\":0,\"alpha\":1,\"last\":2,\"gain\":3}" );
    check( json_equal( json_object_get( report, "registers" ), expected ), "append preserves explicit indices" );
    json_decref( expected );
    json_decref( report );
    static const char * const INVALID[] = { "{\"name\":\"speed\"}", "{\"name\":\"zeta\"}" };

    for( size_t index = 0; index != 2; ++index )
    {
        check( json_array_append_new( registers, parse( INVALID[index] ) ) == 0, "append invalid register" );
        json_decref( compile( document, _binary, 0 ) );
        json_array_remove( registers, json_array_size( registers ) - 1 );
    }

    json_decref( document );
}
//////////////////////////////////////////////////////////////////////////
static void test_invalid( const char * _binary )
{
    static const char * const CHANNELS[] = {
        "{\"custom.a\":{\"expression\":\"ref(\\\"Main.control.custom.a\\\")\"}}",
        "{\"custom.a\":{\"expression\":\"ref(\\\"Main.control.custom.b\\\")\"},\"custom.b\":{\"expression\":\"ref(\\\"Main.control.custom.a\\\")\"}}",
        "{\"rotation\":{\"expression\":\"[1,2]\"}}",
        "{\"position\":{\"expression\":\"[1,2,3] + [1,2]\"}}",
        "{\"rotation\":{\"expression\":\"params.missing\"}}",
        "{\"rotation\":{\"expression\":\"sin(1,2)\"}}",
        "{\"rotation\":{\"expression\":123}}",
        "{\"rotation\":{\"keys\":[{\"time\":2,\"value\":1},{\"time\":1,\"value\":2}]}}",
        "{\"rotation\":{\"keys\":[{\"time\":0,\"value\":0,\"out\":{\"time\":3,\"value\":1}},{\"time\":2,\"value\":2}]}}",
        "{\"rotation\":{\"samples\":[1,2],\"keys\":[{\"time\":0,\"value\":1}]}}",
        "{\"shader.missingBinding\":1}",
        "{\"rotation\":{\"keyz\":[]}}" };

    for( size_t index = 0; index != sizeof( CHANNELS ) / sizeof( CHANNELS[0] ); ++index )
    {
        json_t * document = base_document();
        set_json( first_layer( document ), "channels", CHANNELS[index] );
        json_decref( compile( document, _binary, 0 ) );
        json_decref( document );
    }

    json_t * document = base_document();
    char nested[165];
    memset( nested, '(', 80 );
    memcpy( nested + 80, "time", 4 );
    memset( nested + 84, ')', 80 );
    nested[164] = '\0';
    json_t * expression = json_pack( "{s:{s:s}}", "rotation", "expression", nested );
    check( expression != NULL, "nested expression fixture" );
    json_object_set_new( first_layer( document ), "channels", expression );
    json_decref( compile( document, _binary, 0 ) );
    json_decref( document );
    static const char * const FIELDS[] = { "parent", "composition" };
    static const char * const VALUES[] = { "control", "Main" };

    for( size_t index = 0; index != 2; ++index )
    {
        document = base_document();
        json_object_set_new( first_layer( document ), FIELDS[index], json_string( VALUES[index] ) );
        json_decref( compile( document, _binary, 0 ) );
        json_decref( document );
    }

    char source[4096];
    path( source, sizeof( source ), "check.movie" );
    FILE * file = fopen( source, "wb" );
    check( file != NULL, "duplicate field fixture" );
    fputs( "{\"version\":1,\"version\":1}", file );
    fclose( file );
    json_decref( run( "validate", source, NULL, 0, 0 ) );
}
//////////////////////////////////////////////////////////////////////////
static void test_geometry( const char * _binary )
{
    char assetPath[4096];
    path( assetPath, sizeof( assetPath ), "asset.bin" );
    FILE * asset = fopen( assetPath, "wb" );
    check( asset != NULL, "resource fixture" );
    fputs( "fixture", asset );
    fclose( asset );
    json_t * document = base_document();
    set_json( document, "resources", "{\"texture\":{\"type\":\"image\",\"path\":\"asset.bin\",\"size\":[100,100]}}" );
    json_t * compositions = json_object_get( document, "compositions" );
    json_t * main = json_array_get( compositions, 0 );
    set_json( main, "layers", "[{\"id\":\"mesh\",\"resource\":\"texture\",\"mesh\":{\"vertices\":[[0,0],[100,0],[100,100],[0,100]],\"uvs\":[[0,0],[1,0],[1,1],[0,1]],\"indices\":[0,1,2,0,2,3],\"keys\":[{\"time\":0,\"vertices\":[[0,0],[100,0],[100,100],[0,100]]},{\"time\":4,\"vertices\":[[0,0],[120,0],[100,100],[0,100]]}]}},{\"id\":\"nested\",\"composition\":\"Child\"}]" );
    json_t * child = parse( "{\"id\":\"Child\",\"size\":[100,100],\"duration\":2,\"layers\":[{\"id\":\"null\",\"channels\":{\"rotation\":20}}]}" );
    check( json_array_append_new( compositions, child ) == 0, "child composition fixture" );
    json_decref( compile( document, _binary, 1 ) );
    json_decref( run( "sample", _binary, NULL, 0, 1 ) );
    const char * childOptions[] = { "--composition", "Child", "--time", "0.5" };
    json_decref( run( "sample", _binary, childOptions, 4, 1 ) );
    json_t * resources = json_object_get( document, "resources" );
    set_json( resources, "snow", "{\"type\":\"particle\",\"path\":\"asset.bin\",\"images\":[\"texture\"]}" );
    set_json( resources, "chime", "{\"type\":\"sound\",\"path\":\"asset.bin\",\"duration\":1}" );
    set_json( resources, "sequence", "{\"type\":\"sequence\",\"images\":[\"texture\"],\"fps\":30}" );
    set_json( resources, "video", "{\"type\":\"video\",\"path\":\"asset.bin\",\"size\":[100,100],\"duration\":1,\"fps\":30}" );
    json_decref( compile( document, _binary, 1 ) );
    json_decref( run( "sample", _binary, NULL, 0, 1 ) );
    size_t binarySize;
    unsigned char * bytes = read_bytes( _binary, &binarySize );
    bytes[binarySize - 2] ^= 1;
    FILE * corrupted = fopen( _binary, "wb" );
    check( corrupted != NULL, "corrupt extension fixture" );
    check( fwrite( bytes, 1, binarySize, corrupted ) == binarySize, "write corrupt extension" );
    fclose( corrupted );
    free( bytes );
    json_decref( run( "inspect", _binary, NULL, 0, 0 ) );
    json_t * masks = json_deep_copy( document );
    json_t * masked = json_array_get( json_object_get( masks, "compositions" ), 0 );
    set_json( masked, "layers", "[{\"id\":\"target\",\"resource\":\"texture\",\"matte\":\"mask\"},{\"id\":\"mask\",\"resource\":\"texture\",\"isMatte\":true}]" );
    json_decref( compile( masks, _binary, 1 ) );
    json_decref( run( "sample", _binary, NULL, 0, 1 ) );
    json_t * mask = json_array_get( json_object_get( masked, "layers" ), 1 );
    json_object_set_new( mask, "isMatte", json_false() );
    json_decref( compile( masks, _binary, 0 ) );
    json_decref( masks );
    json_t * warped = json_deep_copy( document );
    json_t * warpedMain = json_array_get( json_object_get( warped, "compositions" ), 0 );
    set_json( warpedMain, "layers", "[{\"id\":\"warp\",\"resource\":\"texture\",\"warp\":{\"corners\":[[0,0],[100,0],[100,100],[0,100]],\"beziers\":[[33,0],[67,0],[100,33],[100,67],[67,100],[33,100],[0,67],[0,33]],\"quality\":1}}]" );
    json_t * warp = json_object_get( json_array_get( json_object_get( warpedMain, "layers" ), 0 ), "warp" );
    json_t * keys = json_array();
    check( keys != NULL, "warp keys fixture" );

    for( size_t index = 0; index != 2; ++index )
    {
        json_t * warpKey = json_pack( "{s:f,s:O,s:O}", "time", (double)index * 4.0, "corners", json_object_get( warp, "corners" ), "beziers", json_object_get( warp, "beziers" ) );
        check( warpKey != NULL && json_array_append_new( keys, warpKey ) == 0, "warp key fixture" );
    }

    json_object_set_new( warp, "keys", keys );
    json_decref( compile( warped, _binary, 1 ) );
    json_decref( run( "sample", _binary, NULL, 0, 1 ) );
    json_decref( warped );
    json_t * mesh = json_object_get( json_array_get( json_object_get( main, "layers" ), 0 ), "mesh" );
    json_t * key = json_array_get( json_object_get( mesh, "keys" ), 1 );
    json_t * vertices = json_object_get( key, "vertices" );
    json_array_remove( vertices, json_array_size( vertices ) - 1 );
    json_decref( compile( document, _binary, 0 ) );
    json_decref( document );
}
//////////////////////////////////////////////////////////////////////////
static void test_legacy( const char * _binary )
{
    json_t * document = base_document();
    json_decref( compile( document, _binary, 1 ) );
    json_decref( document );
    size_t size;
    unsigned char * bytes = read_bytes( _binary, &size );
    check( size > 24, "content header" );
    uint32_t contentSize = (uint32_t)bytes[16] | ( (uint32_t)bytes[17] << 8 ) | ( (uint32_t)bytes[18] << 16 ) | ( (uint32_t)bytes[19] << 24 );
    check( contentSize <= size - 24, "content size" );
    size_t extension = 24 + contentSize;
    FILE * file = fopen( _binary, "wb" );
    check( file != NULL, "extension fixture" );
    check( fwrite( bytes, 1, extension, file ) == extension && fputc( 0, file ) == 0, "empty extension list" );
    fclose( file );
    json_t * report = run( "inspect", _binary, NULL, 0, 1 );
    check( json_integer_value( json_object_get( report, "channels" ) ) == 0, "motion extension is optional" );
    json_decref( report );
    file = fopen( _binary, "wb" );
    check( file != NULL, "duplicate extension fixture" );
    check( fwrite( bytes, 1, size - 1, file ) == size - 1 && fwrite( bytes + extension, 1, size - extension, file ) == size - extension, "duplicate motion extension" );
    fclose( file );
    json_decref( run( "inspect", _binary, NULL, 0, 0 ) );
    bytes[4] = 17;
    bytes[8] = 5;
    char legacy[4096];
    path( legacy, sizeof( legacy ), "legacy.aem" );
    file = fopen( legacy, "wb" );
    check( file != NULL, "legacy fixture" );
    check( fwrite( bytes, 1, 16, file ) == 16 && fwrite( bytes + 24, 1, contentSize, file ) == contentSize, "legacy content" );
    fclose( file );
    free( bytes );
    report = run( "inspect", legacy, NULL, 0, 1 );
    check( json_integer_value( json_object_get( report, "major" ) ) == 17 && json_integer_value( json_object_get( report, "minor" ) ) == 5, "legacy version" );
    check( json_integer_value( json_object_get( report, "channels" ) ) == 0, "legacy has no motion extension" );
    json_decref( report );
}
//////////////////////////////////////////////////////////////////////////
static void test_key( const char * _binary )
{
    json_t * document = base_document();
    json_decref( compile( document, _binary, 1 ) );
    json_decref( document );
    char keyFile[4096];
    char source[4096];
    path( keyFile, sizeof( keyFile ), "key.json" );
    path( source, sizeof( source ), "check.movie" );
    json_t * config = parse( "{\"MoviePlugin\":{\"HASHKEY\":\"0123456789abcdef0123456789abcdef01234567\"}}" );
    check( json_dump_file( config, keyFile, 0 ) == 0, "write key fixture" );
    json_decref( config );
    const char * buildOptions[] = { "-o", _binary, "--key-file", keyFile };
    json_decref( run( "build", source, buildOptions, 4, 1 ) );
    const char * readOptions[] = { "--key-file", keyFile };
    json_decref( run( "sample", _binary, readOptions, 2, 1 ) );
    json_decref( run( "inspect", _binary, NULL, 0, 0 ) );
}
//////////////////////////////////////////////////////////////////////////
int main( int _argc, char * _argv[] )
{
    check( _argc == 2, "test requires a scratch directory" );
    TEST_DIRECTORY = _argv[1];
    char binary[4096];
    path( binary, sizeof( binary ), "check.aem" );
    test_channels( binary );
    test_registers( binary );
    test_invalid( binary );
    test_geometry( binary );
    test_legacy( binary );
    test_key( binary );
    puts( "PASS compiler: formats, determinism, keys, expressions, registers, cycles, samples, geometry, nested graph" );

    return EXIT_SUCCESS;
}
