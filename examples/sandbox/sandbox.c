#include "movie/movie.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <memory.h>

//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_voidptr_t stdlib_movie_alloc( ae_voidptr_t _data, ae_size_t _size )
{
    AE_UNUSED( _data );

    return malloc( _size );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_voidptr_t stdlib_movie_alloc_n( ae_voidptr_t _data, ae_size_t _size, ae_size_t _count )
{
    AE_UNUSED( _data );

    ae_size_t total = _size * _count;

    return malloc( total );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t stdlib_movie_free( ae_voidptr_t _data, ae_constvoidptr_t _ptr )
{
    AE_UNUSED( _data );

    free( (ae_voidptr_t)_ptr );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t stdlib_movie_free_n( ae_voidptr_t _data, ae_constvoidptr_t _ptr )
{
    AE_UNUSED( _data );

    free( (ae_voidptr_t)_ptr );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t stdlib_movie_logerror( ae_voidptr_t _data, aeMovieErrorCode _code, const ae_char_t * _format, ... )
{
    AE_UNUSED( _data );
    AE_UNUSED( _code );

    va_list argList;

    va_start( argList, _format );
    vprintf( _format, argList );
    va_end( argList );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_size_t __read_file( ae_voidptr_t _data, ae_voidptr_t _buff, ae_size_t _carriage, ae_size_t _size )
{
    AE_UNUSED( _carriage );

    FILE * f = (FILE *)_data;

    ae_size_t s = fread( _buff, 1, _size, f );

    return s;
}

AE_CALLBACK ae_void_t __memory_copy( ae_voidptr_t _data, ae_constvoidptr_t _src, ae_voidptr_t _dst, ae_size_t _size )
{
    AE_UNUSED( _data );

    memcpy( _dst, _src, _size );
}

AE_CALLBACK ae_bool_t __resource_provider( const aeMovieResource * _resource, ae_voidptrptr_t _rd, ae_voidptr_t _ud )
{
    AE_UNUSED( _resource );
    AE_UNUSED( _rd );
    AE_UNUSED( _ud );

    return AE_TRUE;
}

AE_CALLBACK ae_void_t __resource_deleter( aeMovieResourceTypeEnum _type, ae_voidptr_t _data, ae_voidptr_t _ud )
{
    AE_UNUSED( _type );
    AE_UNUSED( _data );
    AE_UNUSED( _ud );
}

int main( int argc, char *argv[] )
{
    AE_UNUSED( argc );
    AE_UNUSED( argv );

    const aeMovieInstance * instance = ae_create_movie_instance( "0e41faff7d430be811df87466106e7a9b36cc3ea", &stdlib_movie_alloc, &stdlib_movie_alloc_n, &stdlib_movie_free, &stdlib_movie_free_n, (ae_movie_strncmp_t)AE_NULL, &stdlib_movie_logerror, AE_NULL );

    aeMovieDataProviders data_providers;
    ae_clear_movie_data_providers( &data_providers );

    data_providers.resource_provider = &__resource_provider;
    data_providers.resource_deleter = &__resource_deleter;

    aeMovieData * movie_data = ae_create_movie_data( instance, &data_providers, AE_NULL );

    FILE * f = fopen( "ui.aem", "rb" );

    if( f == NULL )
    {
        return 0;
    }

    aeMovieStream * movie_stream = ae_create_movie_stream( instance, &__read_file, &__memory_copy, f );

    ae_uint32_t load_major_version;
    ae_uint32_t load_minor_version;
    ae_result_t load_movie_data_result = ae_load_movie_data( movie_data, movie_stream, &load_major_version, &load_minor_version );

	if( load_movie_data_result != AE_RESULT_SUCCESSFUL )
	{
        const ae_char_t * load_movie_data_result_info = ae_get_result_string_info( load_movie_data_result );
        printf( "%s\n", load_movie_data_result_info );
        printf( "...failed.\n" );

        ae_delete_movie_data( movie_data );
        ae_delete_movie_stream( movie_stream );
        fclose( f );

        return 0;
	}

    ae_delete_movie_stream( movie_stream );

    fclose( f );

    const aeMovieCompositionData * compositionData = ae_get_movie_composition_data( movie_data, "BigWin" );

    if( compositionData == AE_NULL )
    {
        return 0;
    }

    aeMovieCompositionProviders providers;
    memset( &providers, 0, sizeof( providers ) );

    const aeMovieComposition * composition = ae_create_movie_composition( movie_data, compositionData, AE_TRUE, &providers, AE_NULL );

    //while( 1 )
    //{
    //	ae_update_movie_composition( composition, 150.f );

    //	ae_uint32_t mesh_iterator = 0;

    //	aeMovieRenderMesh mesh;
    //	while( ae_compute_movie_mesh( composition, &mesh_iterator, &mesh ) == AE_TRUE )
    //	{	
    //		printf( "a" );
    //	}

    //	printf( "SUCCESSFUL!!\n" );
    //}

    ae_delete_movie_composition( composition );

    ae_delete_movie_data( movie_data );

    ae_delete_movie_instance( instance );

    return 0;
}
