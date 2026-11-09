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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct TestContext
{
    ae_size_t allocations;
    ae_size_t initialAllocations;
    ae_uint32_t resourceCalls;
    ae_uint32_t rejectCall;
    ae_uint32_t resources;
    ae_uint32_t cacheCalls;
    ae_uint32_t rejectCacheCall;
    ae_uint32_t cachedUVs;
    ae_bool_t cacheAvailable;
} TestContext;

static ae_voidptr_t allocateMemory( TestContext * _context, ae_size_t _size )
{
    ae_voidptr_t memory = malloc( _size );

    if( memory != AE_NULLPTR )
    {
        ++_context->allocations;
    }

    return memory;
}

#ifdef AE_MOVIE_MEMORY_DEBUG
static ae_voidptr_t allocate( ae_userdata_t _userdata, ae_size_t _size, const ae_char_t * _file, ae_uint32_t _line )
#else
static ae_voidptr_t allocate( ae_userdata_t _userdata, ae_size_t _size )
#endif
{
#ifdef AE_MOVIE_MEMORY_DEBUG
    AE_UNUSED( _file );
    AE_UNUSED( _line );
#endif

    ae_voidptr_t memory = allocateMemory( (TestContext *)_userdata, _size );

    return memory;
}

#ifdef AE_MOVIE_MEMORY_DEBUG
static ae_voidptr_t allocateArray( ae_userdata_t _userdata, ae_size_t _size, ae_size_t _count, const ae_char_t * _file, ae_uint32_t _line )
#else
static ae_voidptr_t allocateArray( ae_userdata_t _userdata, ae_size_t _size, ae_size_t _count )
#endif
{
#ifdef AE_MOVIE_MEMORY_DEBUG
    AE_UNUSED( _file );
    AE_UNUSED( _line );
#endif

    ae_size_t size = _size * _count;
    ae_voidptr_t memory = allocateMemory( (TestContext *)_userdata, size );

    return memory;
}

static ae_void_t freeMemory( ae_userdata_t _userdata, ae_constvoidptr_t _memory )
{
    if( _memory == AE_NULLPTR )
    {
        return;
    }

    TestContext * context = (TestContext *)_userdata;
    --context->allocations;

    free( (ae_voidptr_t)_memory );
}

static ae_size_t readFile( ae_voidptr_t _buffer, ae_size_t _carriage, ae_size_t _size, ae_userdata_t _userdata )
{
    AE_UNUSED( _carriage );

    ae_size_t size = fread( _buffer, 1, _size, (FILE *)_userdata );

    return size;
}

static ae_void_t copyMemory( ae_constvoidptr_t _source, ae_voidptr_t _destination, ae_size_t _size, ae_userdata_t _userdata )
{
    AE_UNUSED( _userdata );

    memcpy( _destination, _source, _size );
}

static ae_bool_t provideResource( const aeMovieResource * _resource, ae_userdataptr_t _userdata, ae_userdata_t _context )
{
    AE_UNUSED( _resource );

    TestContext * context = (TestContext *)_context;
    ++context->resourceCalls;

    if( context->resourceCalls == context->rejectCall )
    {
        return AE_FALSE;
    }

    *_userdata = malloc( 1 );

    if( *_userdata == AE_NULLPTR )
    {
        abort();
    }

    ++context->resources;

    return AE_TRUE;
}

static ae_void_t deleteResource( aeMovieResourceTypeEnum _type, ae_voidptr_t _userdata, ae_userdata_t _context )
{
    AE_UNUSED( _type );

    if( _userdata == AE_NULLPTR )
    {
        abort();
    }

    TestContext * context = (TestContext *)_context;
    --context->resources;

    free( _userdata );
}

static ae_bool_t cacheAvailable( const aeMovieDataCacheUVAvailableCallbackData * _callbackData, ae_userdata_t _context )
{
    AE_UNUSED( _callbackData );

    TestContext * context = (TestContext *)_context;

    return context->cacheAvailable;
}

static ae_bool_t provideCachedUV( const aeMovieDataCacheUVProviderCallbackData * _callbackData, ae_userdataptr_t _userdata, ae_userdata_t _context )
{
    AE_UNUSED( _callbackData );

    TestContext * context = (TestContext *)_context;
    ++context->cacheCalls;

    if( context->cacheCalls == context->rejectCacheCall )
    {
        return AE_FALSE;
    }

    *_userdata = malloc( 1 );

    if( *_userdata == AE_NULLPTR )
    {
        abort();
    }

    ++context->cachedUVs;

    return AE_TRUE;
}

static ae_void_t deleteCachedUV( const aeMovieDataCacheUVDeleterCallbackData * _callbackData, ae_userdata_t _context )
{
    ae_userdata_t userdata = _callbackData->uv_cache_userdata;

    if( userdata == AE_NULLPTR )
    {
        return;
    }

    TestContext * context = (TestContext *)_context;
    --context->cachedUVs;

    free( userdata );
}

static ae_bool_t checkLoad( const aeMovieInstance * _instance, TestContext * _context, const aeMovieDataProviders * _providers, const ae_char_t * _path, ae_uint32_t _rejectCall, ae_uint32_t _rejectCacheCall )
{
    _context->resourceCalls = 0;
    _context->rejectCall = _rejectCall;
    _context->cacheCalls = 0;
    _context->rejectCacheCall = _rejectCacheCall;

    FILE * file = fopen( _path, "rb" );

    if( file == NULL )
    {
        return AE_FALSE;
    }

    aeMovieData * data = ae_create_movie_data( _instance, _providers, _context );
    if( data == AE_NULLPTR )
    {
        fclose( file );

        return AE_FALSE;
    }

    aeMovieStream * stream = ae_create_movie_stream( _instance, &readFile, &copyMemory, file );
    if( stream == AE_NULLPTR )
    {
        ae_delete_movie_data( data );
        fclose( file );

        return AE_FALSE;
    }

    ae_uint32_t major;
    ae_uint32_t minor;
    ae_result_t result = ae_load_movie_data( data, stream, &major, &minor );

    ae_delete_movie_stream( stream );
    fclose( file );
    ae_delete_movie_data( data );

    ae_result_t expected = _rejectCall == 0 ? AE_RESULT_SUCCESSFUL : AE_RESULT_NOT_FOUND_RESOURCE;

    if( _rejectCacheCall != 0 )
    {
        expected = AE_RESULT_INTERNAL_ERROR;
    }

    if( result != expected )
    {
        return AE_FALSE;
    }

    if( _context->allocations != _context->initialAllocations )
    {
        fprintf( stderr, "resource rejection %u UV rejection %u left %zu library allocations\n", _rejectCall, _rejectCacheCall, (size_t)(_context->allocations - _context->initialAllocations) );

        return AE_FALSE;
    }

    if( _context->resources != 0 )
    {
        return AE_FALSE;
    }

    if( _context->cachedUVs != 0 )
    {
        return AE_FALSE;
    }

    return AE_TRUE;
}

int main( int _argc, char * _argv[] )
{
    if( _argc != 2 )
    {
        return EXIT_FAILURE;
    }

    TestContext context = {0};

    const aeMovieInstance * instance = ae_create_movie_instance( AE_HASHKEY_EMPTY, &allocate, &allocateArray, &freeMemory, &freeMemory, NULL, NULL, &context );

    if( instance == AE_NULLPTR )
    {
        return EXIT_FAILURE;
    }

    context.initialAllocations = context.allocations;

    aeMovieDataProviders providers;
    ae_clear_movie_data_providers( &providers );

    providers.resource_provider = &provideResource;
    providers.resource_deleter = &deleteResource;
    providers.cache_uv_available = &cacheAvailable;
    providers.cache_uv_provider = &provideCachedUV;
    providers.cache_uv_deleter = &deleteCachedUV;

    const ae_char_t * examples[] = {"Bridge", "Knight", "Peacock", "Unicorn"};
    ae_uint32_t cases = 0;
    ae_bool_t successful = AE_TRUE;

    for( ae_uint32_t example = 0; example != sizeof( examples ) / sizeof( examples[0] ); ++example )
    {
        ae_char_t path[1024];
        int length = snprintf( path, sizeof( path ), "%s/../examples/resources/%s/%s.aem", _argv[1], examples[example], examples[example] );

        if( length < 0 )
        {
            successful = AE_FALSE;
            break;
        }

        if( (size_t)length >= sizeof( path ) )
        {
            successful = AE_FALSE;
            break;
        }

        for( ae_uint32_t cache = 0; cache != 2; ++cache )
        {
            context.cacheAvailable = cache == 0 ? AE_FALSE : AE_TRUE;

            if( checkLoad( instance, &context, &providers, path, 0, 0 ) == AE_FALSE )
            {
                successful = AE_FALSE;
                break;
            }

            ae_uint32_t resourceCount = context.resourceCalls;
            ae_uint32_t cacheCount = context.cacheCalls;
            ++cases;

            for( ae_uint32_t reject = 1; reject <= resourceCount; ++reject )
            {
                if( checkLoad( instance, &context, &providers, path, reject, 0 ) == AE_FALSE )
                {
                    successful = AE_FALSE;
                    break;
                }

                ++cases;
            }

            if( successful == AE_FALSE )
            {
                break;
            }

            for( ae_uint32_t reject = 1; reject <= cacheCount; ++reject )
            {
                if( checkLoad( instance, &context, &providers, path, 0, reject ) == AE_FALSE )
                {
                    successful = AE_FALSE;
                    break;
                }

                ++cases;
            }

            if( successful == AE_FALSE )
            {
                break;
            }

            if( checkLoad( instance, &context, &providers, path, 0, 0 ) == AE_FALSE )
            {
                successful = AE_FALSE;
                break;
            }

            ++cases;
        }

        if( successful == AE_FALSE )
        {
            break;
        }
    }

    ae_delete_movie_instance( instance );

    if( successful == AE_FALSE )
    {
        return EXIT_FAILURE;
    }

    if( context.allocations != 0 )
    {
        return EXIT_FAILURE;
    }

    printf( "PASS %u resource rejection, UV rejection, retry and successful-load cases\n", cases );

    return EXIT_SUCCESS;
}
