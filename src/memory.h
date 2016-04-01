#	ifndef MOVIE_MEMORY_H_
#	define MOVIE_MEMORY_H_

#	include <movie/movie_type.h>

#	define NEW(instance, type) ((type *)instance->memory_alloc(instance->memory_data, sizeof(type)))
#	define NEWN(instance, type, n) ((type *)instance->memory_alloc(instance->memory_data, sizeof(type) * n))
#	define DELETE(instance, ptr) (instance->memory_free(instance->memory_data, ptr))

#	endif