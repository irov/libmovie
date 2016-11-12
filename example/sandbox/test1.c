#	include "movie/movie.h"

#	include <malloc.h>
#	include <stdio.h>

//////////////////////////////////////////////////////////////////////////
static void * stdlib_movie_alloc( void * _data, uint32_t _size )
{
	(void)_data;

	return malloc( _size );
}
//////////////////////////////////////////////////////////////////////////
static void * stdlib_movie_alloc_n( void * _data, uint32_t _size, uint32_t _count )
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
static size_t read_file( void * _data, void * _buff, uint32_t _size )
{
	FILE * f = (FILE *)_data;

	size_t s = fread( _buff, 1, _size, f );

	return s;
}

static void memory_copy( void * _data, const void * _src, void * _dst, size_t _size )
{
	memcpy( _dst, _src, _size );
}

static void * resource_provider( const aeMovieResource * _resource, void * _data )
{
	return AE_NULL;
}

int main( int argc, char *argv[] )
{
	aeMovieInstance * instance = ae_create_movie_instance( &stdlib_movie_alloc, &stdlib_movie_alloc_n, &stdlib_movie_free, &stdlib_movie_free_n, AE_NULL, AE_NULL, AE_NULL, AE_NULL );

	aeMovieData * movieData = ae_create_movie_data( instance );

	FILE * f = fopen( "ui.aem", "rb" );

	if( f == NULL )
	{
		return 0;
	}

	aeMovieStream stream;
	stream.memory_read = &read_file;
	stream.memory_copy = &memory_copy;
	stream.data = f;

	if( ae_load_movie_data( movieData, &stream, &resource_provider, AE_NULL ) == AE_MOVIE_FAILED )
	{
		return 0;
	}

	fclose( f );

	const aeMovieCompositionData * compositionData = ae_get_movie_composition_data( movieData, "BigWin" );

	if( compositionData == AE_NULL )
	{
		return 0;
	}

	aeMovieCompositionProviders providers;

	providers.camera_provider = AE_NULL;

	providers.node_provider = AE_NULL;
	providers.node_destroyer = AE_NULL;
	providers.node_update = AE_NULL;
	providers.track_matte_update = AE_NULL;

	providers.event = AE_NULL;

	providers.composition_state = AE_NULL;

	aeMovieComposition * composition = ae_create_movie_composition( movieData, compositionData, &providers, AE_NULL );


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