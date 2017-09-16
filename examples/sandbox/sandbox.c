#	include "movie/movie.h"

#	include <malloc.h>
#	include <stdio.h>
#	include <stdarg.h>
#	include <memory.h>

//////////////////////////////////////////////////////////////////////////
static ae_voidptr_t stdlib_movie_alloc( ae_voidptr_t _data, ae_size_t _size )
{
	(void)_data;

	return malloc( _size );
}
//////////////////////////////////////////////////////////////////////////
static ae_voidptr_t stdlib_movie_alloc_n( ae_voidptr_t _data, ae_size_t _size, ae_size_t _count )
{
	(void)_data;

	ae_uint32_t total = _size * _count;

	return malloc( total );
}
//////////////////////////////////////////////////////////////////////////
static void stdlib_movie_free( ae_voidptr_t _data, ae_constvoidptr_t _ptr )
{
	(void)_data;

	free( (ae_voidptr_t)_ptr );
}
//////////////////////////////////////////////////////////////////////////
static void stdlib_movie_free_n( ae_voidptr_t _data, ae_constvoidptr_t _ptr )
{
	(void)_data;

	free( (ae_voidptr_t)_ptr );
}
//////////////////////////////////////////////////////////////////////////
static void stdlib_movie_logerror( ae_voidptr_t _data, aeMovieErrorCode _code, const ae_char_t * _format, ... )
{
	(void)_data;
	(void)_code;

	va_list argList;

	va_start( argList, _format );
	vprintf( _format, argList );
	va_end( argList );
}
//////////////////////////////////////////////////////////////////////////
static ae_size_t __read_file( ae_voidptr_t _data, ae_voidptr_t _buff, ae_size_t _carriage, ae_uint32_t _size )
{
    (void)_carriage;
	FILE * f = (FILE *)_data;

	ae_size_t s = fread( _buff, 1, _size, f );

	return s;
}

static void __memory_copy( ae_voidptr_t _data, ae_constvoidptr_t _src, ae_voidptr_t _dst, ae_size_t _size )
{
	(void)_data;

	memcpy( _dst, _src, _size );
}

static ae_voidptr_t resource_provider( const aeMovieResource * _resource, ae_voidptr_t _data )
{
	(void)_resource;
	(void)_data;

	return AE_NULL;
}

int main( int argc, char *argv[] )
{
	(void)argc;
	(void)argv;

	aeMovieInstance * instance = ae_create_movie_instance( "0e41faff7d430be811df87466106e7a9b36cc3ea", &stdlib_movie_alloc, &stdlib_movie_alloc_n, &stdlib_movie_free, &stdlib_movie_free_n, (ae_movie_strncmp_t)AE_NULL, &stdlib_movie_logerror, AE_NULL );

	aeMovieData * movieData = ae_create_movie_data( instance, &resource_provider, AE_NULL, AE_NULL );

	FILE * f = fopen( "ui.aem", "rb" );

	if( f == NULL )
	{
		return 0;
	}

	aeMovieStream * stream = ae_create_movie_stream( instance, &__read_file, &__memory_copy, f );
	//stream.instance = instance;
	//stream.memory_read = &read_file;
	//stream.memory_copy = &memory_copy;
	//stream.data = f;

	if( ae_load_movie_data( movieData, stream ) == AE_MOVIE_FAILED )
	{
		return 0;
	}

	ae_delete_movie_stream( stream );

	fclose( f );

	const aeMovieCompositionData * compositionData = ae_get_movie_composition_data( movieData, "BigWin" );

	if( compositionData == AE_NULL )
	{
		return 0;
	}

    aeMovieCompositionProviders providers;
    memset( &providers, 0, sizeof( providers ) );

	aeMovieComposition * composition = ae_create_movie_composition( movieData, compositionData, AE_TRUE, &providers, AE_NULL );
    
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

	ae_delete_movie_data( movieData );

	ae_delete_movie_instance( instance );

	return 0;
}