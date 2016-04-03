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
static void stdlib_movie_free( void * _data, void * _ptr )
{
	(void)_data;

	free( _ptr );
}
//////////////////////////////////////////////////////////////////////////
static void read_file( void * _data, void * _buff, uint32_t _size )
{
	FILE * f = (FILE *)_data;

	fread( _buff, _size, 1, f );
}

int main()
{
	FILE * f = fopen( "../example/sandbox/Movie_02_Sad.aem", "rb" );

	aeMovieInstance instance;
	make_movie_instance( &instance, &stdlib_movie_alloc, &stdlib_movie_free, AE_NULL );

	aeMovieData * movieData = create_movie_data( &instance );

	aeMovieStream stream;
	stream.read = &read_file;
	stream.data = f;

	if( load_movie_data( &instance, &stream, movieData ) == AE_MOVIE_FAILED )
	{
		return 0;
	}
	
	return 0;
}