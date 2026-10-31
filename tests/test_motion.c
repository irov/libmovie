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

#include "movie/movie.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

static size_t allocations;

static void * allocate( void * _userdata, size_t _size )
{
    (void)_userdata;
    void * value = malloc( _size );

    if( value )
    {
        ++allocations;
    }

    return value;
}

static void * allocate_many( void * _userdata, size_t _size, size_t _count )
{
    return allocate( _userdata, _size * _count );
}

static void release( void * _userdata, const void * _memory )
{
    (void)_userdata;

    if( _memory )
    {
        --allocations;
        free( (void *)_memory );
    }
}

static void copy_memory( const void * _source, void * _destination, size_t _size, void * _userdata )
{
    (void)_userdata;
    memcpy( _destination, _source, _size );
}

typedef struct TestStream
{
    const unsigned char * bytes;
    size_t size;
} TestStream;

static size_t read_memory( void * _output, size_t _position, size_t _size, void * _userdata )
{
    TestStream * input = (TestStream *)_userdata;

    if( _position > input->size || _size > input->size - _position )
    {
        memset( _output, 0, _size );

        return 0;
    }
    memcpy( _output, input->bytes + _position, _size );

    return _size;
}

#define CHECK( condition )                                                              \
    do                                                                                  \
    {                                                                                   \
        if( !( condition ) )                                                            \
        {                                                                               \
            fprintf( stderr, "motion check failed at %d: %s\n", __LINE__, #condition ); \
                                                                                        \
            return EXIT_FAILURE;                                                        \
        }                                                                               \
    } while( 0 )

static ae_bool_t node_provider( const aeMovieNodeProviderCallbackData * _data, ae_voidptrptr_t _node, ae_userdata_t _userdata )
{
    (void)_data;
    (void)_userdata;
    *_node = (void *)1;

    return AE_TRUE;
}

static unsigned start_count, apply_count, finish_count;

static void marker( const aeMovieMarkerCallbackData * _data, void * _userdata )
{
    (void)_userdata;

    if( strcmp( _data->name, "start" ) == 0 )
    {
        ++start_count;
    }

    if( strcmp( _data->name, "apply" ) == 0 )
    {
        ++apply_count;
    }

    if( strcmp( _data->name, "finish" ) == 0 )
    {
        ++finish_count;
    }
}

static int resource_lifetime( const char * _path )
{
    FILE * file = fopen( _path, "rb" );
    long size;
    unsigned char * bytes;
    TestStream input;
    aeMovieDataProviders data_providers;
    aeMovieCompositionProviders providers;
    ae_uint32_t major, minor, index;
    const aeMovieInstance * instance;
    aeMovieData * movie;
    aeMovieStream * stream;
    CHECK( file != NULL );
    fseek( file, 0, SEEK_END );
    size = ftell( file );
    rewind( file );
    CHECK( size > 0 );
    bytes = (unsigned char *)malloc( (size_t)size );
    CHECK( bytes != NULL );
    CHECK( fread( bytes, 1, (size_t)size, file ) == (size_t)size );
    fclose( file );
    input.bytes = bytes;
    input.size = (size_t)size;
    instance = ae_create_movie_instance( AE_HASHKEY_EMPTY, allocate, allocate_many, release, release, NULL, NULL, NULL );
    CHECK( instance != NULL );
    ae_clear_movie_data_providers( &data_providers );
    movie = ae_create_movie_data( instance, &data_providers, NULL );
    CHECK( movie != NULL );
    stream = ae_create_movie_stream( instance, read_memory, copy_memory, &input );
    CHECK( stream != NULL );
    CHECK( ae_load_movie_data( movie, stream, &major, &minor ) == AE_RESULT_SUCCESSFUL );
    ae_delete_movie_stream( stream );
    ae_initialize_movie_composition_providers( &providers );
    providers.node_provider = node_provider;

    for( index = 0; index != ae_get_movie_composition_data_count( movie ); ++index )
    {
        const aeMovieComposition * composition = ae_create_movie_composition( ae_get_movie_composition_data_by_index( movie, index ), AE_TRUE, &providers, NULL );
        CHECK( composition != NULL );
        ae_set_movie_composition_time( composition, .5f );
        ae_delete_movie_composition( composition );
    }
    ae_delete_movie_data( movie );
    ae_delete_movie_instance( instance );
    free( bytes );
    CHECK( allocations == 0 );

    return EXIT_SUCCESS;
}

static int test_motion_inputs( const aeMovieComposition * _first, const aeMovieComposition * _second, const aeMovieData * _movie )
{
    ae_uint32_t index;
    ae_uint32_t iterator = 0;
    aeMovieRenderMesh mesh;
    float value;

    for( index = 0; index != AE_MOVIE_MOTION_PARAMETER_COUNT; ++index )
    {
        aeMovieMotionParameter parameter = (aeMovieMotionParameter)index;
        CHECK( ae_get_movie_motion_parameter_name( parameter ) != NULL );
        CHECK( isfinite( ae_get_movie_motion_parameter_default( parameter ) ) );
    }
    CHECK( ae_get_movie_motion_parameter_name( AE_MOVIE_MOTION_PARAMETER_COUNT ) == NULL );
    CHECK( !ae_set_movie_motion_parameter( _first, AE_MOVIE_MOTION_PARAMETER_COUNT, 1 ) );
    CHECK( !ae_set_movie_motion_parameter( _first, (aeMovieMotionParameter)-1, 1 ) );
    CHECK( !ae_get_movie_motion_parameter( _first, AE_MOVIE_MOTION_PARAMETER_AMPLITUDE, NULL ) );
    CHECK( !ae_set_movie_motion_parameter( _first, AE_MOVIE_MOTION_PARAMETER_INTENSITY, INFINITY ) );
    CHECK( ae_get_movie_motion_parameter( _second, AE_MOVIE_MOTION_PARAMETER_AMPLITUDE, &value ) && value == 12 );
    CHECK( ae_get_movie_motion_parameter( _first, AE_MOVIE_MOTION_PARAMETER_INTENSITY, &value ) && value == 1 );
    ae_set_movie_marker_callback( _first, marker, NULL );
    ae_set_movie_composition_time( _first, .25f );
    ae_pause_movie_composition( _first );
    CHECK( ae_set_movie_motion_parameter( _first, AE_MOVIE_MOTION_PARAMETER_AMPLITUDE, 30 ) );
    CHECK( ae_evaluate_movie_motion_channel( _first, "Demo.controller.position.y", .25f, &value ) && fabsf( value - 230 ) < .001f );
    CHECK( ae_set_movie_motion_parameter( _first, AE_MOVIE_MOTION_PARAMETER_AMPLITUDE, 40 ) );
    CHECK( ae_evaluate_movie_motion_channel( _first, "Demo.controller.position.y", .25f, &value ) && fabsf( value - 240 ) < .001f );
    ae_update_movie_composition( _first, 0, NULL );
    CHECK( ae_compute_movie_mesh( _first, &iterator, &mesh ) && fabsf( mesh.position[0][1] - 240 ) < .001f );
    CHECK( fabsf( ae_get_movie_composition_time( _first ) - .25f ) < .001f );
    CHECK( start_count == 0 && apply_count == 0 && finish_count == 0 );
    CHECK( !ae_set_movie_motion_register( _first, ae_get_movie_motion_register_count( _movie ), 1 ) );
    CHECK( !ae_find_movie_motion_register( _movie, "missing", &index ) );

    if( ae_get_movie_motion_register_count( _movie ) != 0 )
    {
        CHECK( ae_find_movie_motion_register( _movie, "gain", &index ) );
        CHECK( strcmp( ae_get_movie_motion_register_name( _movie, index ), "gain" ) == 0 );
        CHECK( !ae_get_movie_motion_register( _first, index, NULL ) );
        CHECK( !ae_set_movie_motion_register( _first, index, NAN ) );
        CHECK( ae_set_movie_motion_register( _first, index, 8 ) );
        CHECK( ae_set_movie_motion_parameter( _first, AE_MOVIE_MOTION_PARAMETER_INTENSITY, .5f ) );
        CHECK( ae_evaluate_movie_motion_channel( _first, "Demo.controller.custom.gain", .25f, &value ) && value == 4 );
        CHECK( ae_evaluate_movie_motion_channel( _first, "Demo.controller.custom.registerDependent", .25f, &value ) && value == 8 );
        CHECK( ae_set_movie_motion_register( _first, index, 10 ) );
        CHECK( ae_evaluate_movie_motion_channel( _first, "Demo.controller.custom.registerDependent", .25f, &value ) && value == 10 );
        CHECK( ae_get_movie_motion_register( _second, index, &value ) && value == 2 );
        CHECK( ae_set_movie_motion_parameter( _first, AE_MOVIE_MOTION_PARAMETER_INTENSITY, 1 ) );
    }
    ae_set_movie_marker_callback( _first, NULL, NULL );
    ae_play_movie_composition( _first, .25f );
    CHECK( ae_set_movie_motion_parameter( _first, AE_MOVIE_MOTION_PARAMETER_AMPLITUDE, 50 ) );
    ae_update_movie_composition( _first, 0, NULL );
    iterator = 0;
    CHECK( ae_compute_movie_mesh( _first, &iterator, &mesh ) && fabsf( mesh.position[0][1] - 250 ) < .001f );
    ae_stop_movie_composition( _first );
    ae_set_movie_composition_time( _first, 0 );

    return EXIT_SUCCESS;
}

int main( int _argc, char ** _argv )
{
    FILE * file;
    long size;
    unsigned char * bytes;
    const aeMovieInstance * instance;
    aeMovieDataProviders data_providers;
    aeMovieCompositionProviders providers;
    aeMovieData * movie;
    aeMovieStream * stream;
    ae_uint32_t major, minor;
    TestStream input;
    const aeMovieComposition *first, *second;
    float value;
    size_t stable_allocations;

    if( _argc == 3 && strcmp( _argv[1], "--resources" ) == 0 )
    {
        return resource_lifetime( _argv[2] );
    }
    CHECK( _argc == 2 );
    file = fopen( _argv[1], "rb" );
    CHECK( file != NULL );
    fseek( file, 0, SEEK_END );
    size = ftell( file );
    rewind( file );
    bytes = (unsigned char *)malloc( (size_t)size );
    CHECK( bytes != NULL );
    CHECK( fread( bytes, 1, (size_t)size, file ) == (size_t)size );
    fclose( file );
    input.bytes = bytes;
    input.size = (size_t)size;
    instance = ae_create_movie_instance( AE_HASHKEY_EMPTY, allocate, allocate_many, release, release, NULL, NULL, NULL );
    CHECK( instance != NULL );
    ae_clear_movie_data_providers( &data_providers );
    movie = ae_create_movie_data( instance, &data_providers, NULL );
    stream = ae_create_movie_stream( instance, read_memory, copy_memory, &input );
    CHECK( ae_load_movie_data( movie, stream, &major, &minor ) == AE_RESULT_SUCCESSFUL );
    ae_delete_movie_stream( stream );
    CHECK( major == 18 && minor == 0 && ae_is_movie_motion_data( movie ) );
    ae_initialize_movie_composition_providers( &providers );
    providers.node_provider = node_provider;
    first = ae_create_movie_composition( ae_get_movie_composition_data_by_index( movie, 0 ), AE_TRUE, &providers, NULL );
    second = ae_create_movie_composition( ae_get_movie_composition_data_by_index( movie, 0 ), AE_TRUE, &providers, NULL );
    CHECK( first && second );
    {
        const aeMovieComposition * fractional = ae_create_movie_composition( ae_get_movie_composition_data_by_index( movie, 1 ), AE_TRUE, &providers, NULL );
        aeMovieRenderMesh mesh;
        ae_uint32_t iterator = 0;
        CHECK( fractional != NULL );
        ae_set_movie_marker_callback( fractional, marker, NULL );
        ae_play_movie_composition( fractional, 0 );
        CHECK( ae_update_movie_composition( fractional, 1.035f, NULL ) == AE_FALSE );
        CHECK( finish_count == 0 && ae_compute_movie_mesh( fractional, &iterator, &mesh ) == AE_TRUE );
        CHECK( ae_update_movie_composition( fractional, .003f, NULL ) == AE_TRUE && finish_count == 1 );
        ae_set_movie_composition_time( fractional, 1.037f );
        iterator = 0;
        CHECK( ae_compute_movie_mesh( fractional, &iterator, &mesh ) == AE_TRUE && fabsf( mesh.position[0][0] - 5 ) < .001f );
        ae_delete_movie_composition( fractional );
        finish_count = 0;
    }
    stable_allocations = allocations;
    CHECK( test_motion_inputs( first, second, movie ) == EXIT_SUCCESS );
    CHECK( ae_set_movie_motion_parameter( first, AE_MOVIE_MOTION_PARAMETER_AMPLITUDE, 20 ) );
    CHECK( !ae_set_movie_motion_parameter( first, AE_MOVIE_MOTION_PARAMETER_AMPLITUDE, NAN ) );
    CHECK( ae_evaluate_movie_motion_channel( first, "Demo.controller.position.y", .25f, &value ) && fabsf( value - 220 ) < .0001f );
    CHECK( ae_evaluate_movie_motion_channel( second, "Demo.controller.position.y", .25f, &value ) && fabsf( value - 212 ) < .0001f );
    CHECK( ae_evaluate_movie_motion_channel( first, "Demo.controller.rotation", 2, &value ) && fabsf( value - 360 ) < .001f );
    CHECK( ae_evaluate_movie_motion_channel( first, "Demo.controller.custom.follow", 2, &value ) && fabsf( value - 180 ) < .001f );
    CHECK( ae_evaluate_movie_motion_channel( first, "Demo.controller.custom.overshoot", 2, &value ) && value > 2.4f );
    CHECK( ae_evaluate_movie_motion_channel( first, "Demo.controller.rotation", 4, &value ) && value == 720 );
    ae_set_movie_marker_callback( first, marker, NULL );
    {
        aeMovieRenderMesh mesh;
        ae_uint32_t iterator = 0;
        ae_set_movie_composition_time( first, 4 );
        CHECK( ae_compute_movie_mesh( first, &iterator, &mesh ) == AE_TRUE );
        ae_set_movie_composition_time( first, .237f );
        iterator = 0;
        CHECK( ae_compute_movie_mesh( first, &iterator, &mesh ) == AE_TRUE );
        CHECK( ae_evaluate_movie_motion_channel( first, "Demo.controller.position.y", .237f, &value ) );
        CHECK( fabsf( mesh.position[0][1] - value ) < .001f );
        ae_play_movie_composition( first, 0 );
        CHECK( ae_update_movie_composition( first, 3.98f, NULL ) == AE_FALSE );
        CHECK( finish_count == 0 );
        CHECK( ae_update_movie_composition( first, .02f, NULL ) == AE_TRUE );
        CHECK( finish_count == 1 );
        start_count = apply_count = finish_count = 0;
    }

    ae_set_movie_composition_time( first, 3 );
    CHECK( start_count == 0 && apply_count == 0 );
    ae_play_movie_composition( first, 0 );
    ae_update_movie_composition( first, 2, NULL );
    CHECK( start_count == 1 && apply_count == 1 );
    ae_update_movie_composition( first, 0, NULL );
    CHECK( apply_count == 1 );
    ae_pause_movie_composition( first );
    ae_update_movie_composition( first, 1, NULL );
    CHECK( finish_count == 0 );
    ae_resume_movie_composition( first );
    CHECK( ae_finish_movie_motion( first ) );
    CHECK( apply_count == 1 && finish_count == 1 );
    CHECK( ae_finish_movie_motion( first ) );
    CHECK( apply_count == 1 && finish_count == 1 );
    CHECK( allocations == stable_allocations );
    start_count = apply_count = finish_count = 0;
    ae_set_movie_composition_loop( first, AE_TRUE );
    ae_play_movie_composition( first, 0 );
    ae_update_movie_composition( first, 9, NULL );
    CHECK( start_count == 3 && apply_count == 2 && finish_count == 2 );
    ae_delete_movie_composition( first );
    ae_delete_movie_composition( second );
    ae_delete_movie_data( movie );

    input.size = (size_t)size - 1;
    movie = ae_create_movie_data( instance, &data_providers, NULL );
    stream = ae_create_movie_stream( instance, read_memory, copy_memory, &input );
    CHECK( ae_load_movie_data( movie, stream, &major, &minor ) != AE_RESULT_SUCCESSFUL );
    ae_delete_movie_stream( stream );
    ae_delete_movie_data( movie );
    input.size = (size_t)size;
    bytes[size - 2] ^= 1;
    movie = ae_create_movie_data( instance, &data_providers, NULL );
    stream = ae_create_movie_stream( instance, read_memory, copy_memory, &input );
    CHECK( ae_load_movie_data( movie, stream, &major, &minor ) == AE_RESULT_INVALID_DATA );
    ae_delete_movie_stream( stream );
    ae_delete_movie_data( movie );
    bytes[size - 2] ^= 1;
    stable_allocations = allocations;

    {
        long offset;

        for( offset = 0; offset != size; ++offset )
        {
            uint32_t hash = 2166136261U;
            long index;
            size_t contentSize = (size_t)bytes[16] | ( (size_t)bytes[17] << 8 ) | ( (size_t)bytes[18] << 16 ) | ( (size_t)bytes[19] << 24 );
            size_t extension = 24 + contentSize;
            unsigned char savedChecksum[8];
            memcpy( savedChecksum, bytes + 20, 4 );
            memcpy( savedChecksum + 4, bytes + extension + 5, 4 );
            bytes[offset] ^= 0x80;

            for( index = 24; index < (long)extension; ++index )
            {
                hash = ( hash ^ bytes[index] ) * 16777619U;
            }

            if( offset >= 24 && offset < (long)extension )
            {
                for( index = 0; index != 4; ++index )
                {
                    bytes[20 + index] = (unsigned char)( hash >> ( index * 8 ) );
                }
            }
            hash = 2166136261U;

            for( index = (long)extension + 9; index < size - 1; ++index )
            {
                hash = ( hash ^ bytes[index] ) * 16777619U;
            }

            if( offset >= (long)extension + 9 && offset < size - 1 )
            {
                for( index = 0; index != 4; ++index )
                {
                    bytes[extension + 5 + index] = (unsigned char)( hash >> ( index * 8 ) );
                }
            }
            input.size = (size_t)size;
            movie = ae_create_movie_data( instance, &data_providers, NULL );
            stream = ae_create_movie_stream( instance, read_memory, copy_memory, &input );
            (void)ae_load_movie_data( movie, stream, &major, &minor );
            ae_delete_movie_stream( stream );
            ae_delete_movie_data( movie );
            bytes[offset] ^= 0x80;
            memcpy( bytes + 20, savedChecksum, 4 );
            memcpy( bytes + extension + 5, savedChecksum + 4, 4 );
            CHECK( allocations == stable_allocations );
            input.size = (size_t)offset;
            movie = ae_create_movie_data( instance, &data_providers, NULL );
            stream = ae_create_movie_stream( instance, read_memory, copy_memory, &input );
            CHECK( ae_load_movie_data( movie, stream, &major, &minor ) != AE_RESULT_SUCCESSFUL );
            ae_delete_movie_stream( stream );
            ae_delete_movie_data( movie );
            CHECK( allocations == stable_allocations );
        }
    }
    ae_delete_movie_instance( instance );
    free( bytes );
    CHECK( allocations == 0 );

    return EXIT_SUCCESS;
}
