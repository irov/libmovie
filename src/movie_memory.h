#	ifndef MOVIE_MEMORY_H_
#	define MOVIE_MEMORY_H_

#	include <movie/movie_type.h>

#	define NEW(instance, type) ((type *)instance->memory_alloc(instance->instance_data, sizeof(type)))
#	define NEWN(instance, type, n) (n == 0 ? AE_NULL : (type *)instance->memory_alloc_n(instance->instance_data, sizeof(type), n))
#	define DELETE(instance, ptr) (instance->memory_free(instance->instance_data, ptr))
#	define DELETEN(instance, ptr) (instance->memory_free_n(instance->instance_data, ptr))
#	define STRNCMP(instance, src, dst, count) (instance->strncmp(instance->instance_data, src, dst, count))

#	endif