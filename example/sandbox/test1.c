#	include "movie/movie.h"

#	include <stdio.h>

static void read_file( void * _data, void * _buff, uint32_t _size )
{
	FILE * f = (FILE *)_data;

	fread( _buff, _size, 1, f );
}

int main()
{
	FILE * f = fopen( "../example/sandbox/Movie_02_Sad.aem", "rb" );

	aeMovieInstance instance;
	make_movie_instance( &instance, AE_NULL, AE_NULL, AE_NULL );

	aeMovieData * movieData = create_movie_data( &instance );

	aeMovieStream stream;
	stream.read = &read_file;
	stream.data = f;

	if( load_movie_data( &instance, movieData, &stream ) == AE_MOVIE_FAILED )
	{
		return 0;
	}
	
	return 0;
}