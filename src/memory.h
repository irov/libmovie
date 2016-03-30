#	ifndef MOVIE_MEMORY_H_
#	define MOVIE_MEMORY_H_

#	include "movie/type.h"

void * default_movie_alloc( void * _data, uint32_t _size );
void default_movie_free( void * _data, void * _ptr );

#	define nullptr ((void *)0)

#	define NEW(instance, type) ((type *)instance->memory_alloc(instance->memory_data, sizeof(type)))
#	define DELETE(instance, ptr) (instance->memory_free(instance->memory_data, ptr))

#	endif