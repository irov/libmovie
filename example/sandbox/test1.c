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
static void read_file( void * _data, void * _buff, uint32_t _size )
{
	FILE * f = (FILE *)_data;

	fread( _buff, _size, 1, f );
}

static void * resource_provider( const aeMovieResource * _resource, void * _data )
{
	return AE_NULL;
}

int main()
{
	aeMovieInstance instance;
	make_movie_instance( &instance, &stdlib_movie_alloc, &stdlib_movie_alloc_n, &stdlib_movie_free, &stdlib_movie_free_n, AE_NULL );

	aeMovieData * movieData = create_movie_data( &instance );

	FILE * f = fopen( "02_Sad.aem", "rb" );

	if( f == NULL )
	{
		return 0;
	}

	aeMovieStream stream;
	stream.read = &read_file;
	stream.data = f;

	if( load_movie_data( &instance, &stream, movieData, &resource_provider, AE_NULL ) == AE_MOVIE_FAILED )
	{
		return 0;
	}

	fclose( f );

	const aeMovieCompositionData * compositionData = get_movie_composition_data( movieData, "Tuman" );
	
	aeMovieComposition * composition = create_movie_composition( &instance, movieData, compositionData );


	while( 1 )
	{
		update_movie_composition( composition, 150.f );

		aeMovieRenderContext context;
		begin_movie_render_context( composition, &context );

		
		for( uint32_t i = 0; i != context.mesh_count; ++i )
		{			
			aeMovieRenderMesh mesh;
			compute_movie_mesh( &context, i, &mesh );

			printf( "a" );
		}

		printf( "SUCCESSFUL!!\n" );
	}

	destroy_movie_composition( &instance, composition );

	delete_movie_data( &instance, movieData );
	
	return 0;
}