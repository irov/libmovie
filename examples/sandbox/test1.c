#	include "movie/movie.h"

#	include <malloc.h>
#	include <stdio.h>
#	include <stdarg.h>
#	include <memory.h>

//////////////////////////////////////////////////////////////////////////
static void * stdlib_movie_alloc( void * _data, size_t _size )
{
	(void)_data;

	return malloc( _size );
}
//////////////////////////////////////////////////////////////////////////
static void * stdlib_movie_alloc_n( void * _data, size_t _size, size_t _count )
{
	(void)_data;

	uint32_t total = _size * _count;

	return malloc( total );
}
//////////////////////////////////////////////////////////////////////////
static void stdlib_movie_free( void * _data, const void * _ptr )
{
	(void)_data;

	free( (void *)_ptr );
}
//////////////////////////////////////////////////////////////////////////
static void stdlib_movie_free_n( void * _data, const void * _ptr )
{
	(void)_data;

	free( (void *)_ptr );
}
//////////////////////////////////////////////////////////////////////////
static void stdlib_movie_logerror( void * _data, aeMovieErrorCode _code, const char * _format, ... )
{
	(void)_data;
	(void)_code;

	va_list argList;

	va_start( argList, _format );
	vprintf( _format, argList );
	va_end( argList );
}
//////////////////////////////////////////////////////////////////////////
static size_t read_file( void * _data, void * _buff, uint32_t _size )
{
	FILE * f = (FILE *)_data;

	size_t s = fread( _buff, 1, _size, f );

	return s;
}

static void memory_copy( void * _data, const void * _src, void * _dst, size_t _size )
{
	(void)_data;

	memcpy( _dst, _src, _size );
}

static void * resource_provider( const aeMovieResource * _resource, void * _data )
{
	(void)_resource;
	(void)_data;

	return AE_NULL;
}

int main( int argc, char *argv[] )
{
	(void)argc;
	(void)argv;

	aeMovieInstance * instance = ae_create_movie_instance( &stdlib_movie_alloc, &stdlib_movie_alloc_n, &stdlib_movie_free, &stdlib_movie_free_n, (ae_movie_strncmp_t)AE_NULL, &stdlib_movie_logerror, AE_NULL );

	aeMovieData * movieData = ae_create_movie_data( instance );

	FILE * f = fopen( "ui.aem", "rb" );

	if( f == NULL )
	{
		return 0;
	}

	aeMovieStream * stream = ae_create_movie_stream( instance, &read_file, &memory_copy, f );
	//stream.instance = instance;
	//stream.memory_read = &read_file;
	//stream.memory_copy = &memory_copy;
	//stream.data = f;

	if( ae_load_movie_data( movieData, stream, &resource_provider, AE_NULL ) == AE_MOVIE_FAILED )
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

	providers.camera_provider = (ae_movie_callback_camera_provider_t)AE_NULL;

	providers.node_provider = (ae_movie_callback_node_provider_t)AE_NULL;
	providers.node_destroyer = (ae_movie_callback_node_destroy_t)AE_NULL;
	providers.node_update = (ae_movie_callback_node_update_t)AE_NULL;
	providers.track_matte_update = (ae_movie_callback_track_matte_update_t)AE_NULL;

	providers.composition_event = (ae_movie_callback_composition_event_t)AE_NULL;

	providers.composition_state = (ae_movie_callback_composition_state_t)AE_NULL;

	aeMovieComposition * composition = ae_create_movie_composition( movieData, compositionData, AE_TRUE, &providers, AE_NULL );


	//while( 1 )
	//{
	//	ae_update_movie_composition( composition, 150.f );

	//	uint32_t mesh_iterator = 0;

	//	aeMovieRenderMesh mesh;
	//	while( ae_compute_movie_mesh( composition, &mesh_iterator, &mesh ) == AE_TRUE )
	//	{	
	//		printf( "a" );
	//	}

	//	printf( "SUCCESSFUL!!\n" );
	//}

	ae_destroy_movie_composition( composition );

	ae_delete_movie_data( movieData );

	ae_delete_movie_instance( instance );

	return 0;
}