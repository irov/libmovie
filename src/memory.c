#	include "memory.h"

#	include <malloc.h>

void * default_movie_alloc( void * _data, uint32_t _size )
{
	(void)_data;

	return malloc( _size );
}

void default_movie_free( void * _data, void * _ptr )
{
	(void)_data;

	free( _ptr );
}