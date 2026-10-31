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
#include <stdarg.h>
#include <memory.h>

//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_voidptr_t stdlib_movie_alloc( ae_userdata_t _data, ae_size_t _size )
{
    AE_UNUSED( _data );

    return malloc( _size );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_voidptr_t stdlib_movie_alloc_n( ae_userdata_t _data, ae_size_t _size, ae_size_t _count )
{
    AE_UNUSED( _data );

    ae_size_t total = _size * _count;

    return malloc( total );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t stdlib_movie_free( ae_userdata_t _data, ae_constvoidptr_t _ptr )
{
    AE_UNUSED( _data );

    free( (ae_voidptr_t)_ptr );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t stdlib_movie_free_n( ae_userdata_t _data, ae_constvoidptr_t _ptr )
{
    AE_UNUSED( _data );

    free( (ae_voidptr_t)_ptr );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t stdlib_movie_logerror( ae_userdata_t _data, aeMovieErrorCode _code, const ae_char_t * _format, ... )
{
    AE_UNUSED( _data );
    AE_UNUSED( _code );

    va_list argList;

    va_start( argList, _format );
    vprintf( _format, argList );
    va_end( argList );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_size_t __read_file( ae_voidptr_t _buff, ae_size_t _carriage, ae_size_t _size, ae_userdata_t _data )
{
    AE_UNUSED( _carriage );

    FILE * f = (FILE *)_data;

    ae_size_t s = fread( _buff, 1, _size, f );

    return s;
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t __memory_copy( ae_constvoidptr_t _src, ae_voidptr_t _dst, ae_size_t _size, ae_userdata_t _data )
{
    AE_UNUSED( _data );

    memcpy( _dst, _src, _size );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_bool_t __resource_provider( const aeMovieResource * _resource, ae_userdataptr_t _rd, ae_userdata_t _ud )
{
    AE_UNUSED( _resource );
    AE_UNUSED( _rd );
    AE_UNUSED( _ud );

    return AE_TRUE;
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t __resource_deleter( aeMovieResourceTypeEnum _type, ae_voidptr_t _data, ae_userdata_t _ud )
{
    AE_UNUSED( _type );
    AE_UNUSED( _data );
    AE_UNUSED( _ud );
}
//////////////////////////////////////////////////////////////////////////
int main( int argc, char * argv[] )
{
    AE_UNUSED( argc );
    AE_UNUSED( argv );

    const aeMovieInstance * instance = ae_create_movie_instance( AE_HASHKEY_EMPTY
        , &stdlib_movie_alloc
        , &stdlib_movie_alloc_n
        , &stdlib_movie_free
        , &stdlib_movie_free_n
        , (ae_movie_strncmp_t)AE_NULLPTR
        , &stdlib_movie_logerror
        , AE_NULLPTR );

    aeMovieDataProviders data_providers;
    ae_clear_movie_data_providers( &data_providers );

    data_providers.resource_provider = &__resource_provider;
    data_providers.resource_deleter = &__resource_deleter;

    aeMovieData * movie_data = ae_create_movie_data( instance, &data_providers, AE_USERDATA_NULL );

    FILE * f = fopen( "../../../examples/resources/Knight/Knight.aem", "rb" );

    if( f == NULL )
    {
        return EXIT_FAILURE;
    }

    aeMovieStream * movie_stream = ae_create_movie_stream( instance, &__read_file, &__memory_copy, f );

    ae_uint32_t load_major_version;
    ae_uint32_t load_minor_version;
    ae_result_t load_movie_data_result = ae_load_movie_data( movie_data, movie_stream, &load_major_version, &load_minor_version );

    if( load_movie_data_result != AE_RESULT_SUCCESSFUL )
    {
        const ae_char_t * load_movie_data_result_info = ae_get_movie_result_string_info( load_movie_data_result );
        printf( "%s\n", load_movie_data_result_info );
        printf( "...failed.\n" );

        ae_delete_movie_data( movie_data );
        ae_delete_movie_stream( movie_stream );
        fclose( f );

        return EXIT_FAILURE;
    }

    ae_delete_movie_stream( movie_stream );

    fclose( f );

    const aeMovieCompositionData * compositionData = ae_get_movie_composition_data( movie_data, "Knight" );

    if( compositionData == AE_NULLPTR )
    {
        return EXIT_FAILURE;
    }

    aeMovieCompositionProviders providers;
    ae_initialize_movie_composition_providers( &providers );

    const aeMovieComposition * composition = ae_create_movie_composition( compositionData, AE_TRUE, &providers, AE_NULLPTR );

    ae_play_movie_composition( composition, 0.f );

    for( ;; )
    {
        if( ae_update_movie_composition( composition, 0.01f, AE_NULLPTR ) == AE_TRUE )
        {
            break;
        }

        ae_uint32_t mesh_iterator = 0;

        aeMovieRenderMesh mesh;
        while( ae_compute_movie_mesh( composition, &mesh_iterator, &mesh ) == AE_TRUE )
        {
            printf( "layer_type '%d' vertex '%d' index '%d'\n"
                , mesh.layer_type
                , mesh.vertexCount
                , mesh.indexCount
            );
        }
    }

    ae_delete_movie_composition( composition );

    ae_delete_movie_data( movie_data );

    ae_delete_movie_instance( instance );

    return EXIT_SUCCESS;
}
