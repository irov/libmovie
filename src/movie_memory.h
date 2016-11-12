#	ifndef MOVIE_MEMORY_H_
#	define MOVIE_MEMORY_H_

#	include "movie/movie_type.h"

#	include "movie_struct.h"

//////////////////////////////////////////////////////////////////////////
#	ifdef AE_MOVIE_MEMORY_INFO
//////////////////////////////////////////////////////////////////////////
#	define NEW(instance, type) ((type *)ae_magic_memory_alloc_info( instance, #type, sizeof(type)))
#	define NEWN(instance, type, n) (n == 0 ? (type *)AE_NULL : (type *)ae_magic_memory_alloc_n_info(instance, #type, sizeof(type), n))
#	define DELETE(instance, ptr) (ae_magic_memory_free_info(instance, #ptr, ptr))
#	define DELETEN(instance, ptr) (ae_magic_memory_free_n_info(instance, #ptr, ptr))
//////////////////////////////////////////////////////////////////////////
#	else
//////////////////////////////////////////////////////////////////////////
#	define NEW(instance, type) ((type *)ae_magic_memory_alloc( instance, sizeof(type)))
#	define NEWN(instance, type, n) (n == 0 ? (type *)AE_NULL : (type *)ae_magic_memory_alloc_n(instance, sizeof(type), n))
#	define DELETE(instance, ptr) (ae_magic_memory_free(instance, ptr))
#	define DELETEN(instance, ptr) (ae_magic_memory_free_n(instance, ptr))
//////////////////////////////////////////////////////////////////////////
#	endif
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
#	ifdef AE_MOVIE_MEMORY_INFO
//////////////////////////////////////////////////////////////////////////
static void * ae_magic_memory_alloc_info( const aeMovieInstance * _instance, const char * _type, size_t _size )
{
	_instance->logger( _instance->instance_data, AE_ERROR_MEMORY, "alloc type '%s' size '%d'", _type, (uint32_t)_size );

	void * ptr = _instance->memory_alloc( _instance->instance_data, _size );

	return ptr;
}
//////////////////////////////////////////////////////////////////////////
static void * ae_magic_memory_alloc_n_info( const aeMovieInstance * _instance, const char * _type, size_t _size, size_t _count )
{	
	_instance->logger( _instance->instance_data, AE_ERROR_MEMORY, "alloc n type '%s' size '%d' count '%d'", _type, (uint32_t)_size, (uint32_t)_count );

	void * ptr = _instance->memory_alloc_n( _instance->instance_data, _size, _count );

	return ptr;
}
//////////////////////////////////////////////////////////////////////////
static void ae_magic_memory_free_info( const aeMovieInstance * _instance, const char * _type, const void * _ptr )
{
	_instance->logger( _instance->instance_data, AE_ERROR_MEMORY, "free type '%s' ptr '%p'", _type, _ptr );

	_instance->memory_free( _instance->instance_data, _ptr );
}
//////////////////////////////////////////////////////////////////////////
static void ae_magic_memory_free_n_info( const aeMovieInstance * _instance, const char * _type, const void * _ptr )
{
	_instance->logger( _instance->instance_data, AE_ERROR_MEMORY, "free n type '%s' ptr '%p'", _type, _ptr );

	_instance->memory_free_n( _instance->instance_data, _ptr );
}
//////////////////////////////////////////////////////////////////////////
#	else
//////////////////////////////////////////////////////////////////////////
static void * ae_magic_memory_alloc( const aeMovieInstance * _instance, size_t _size )
{
	void * ptr = _instance->memory_alloc( _instance->instance_data, _size );

	return ptr;
}
//////////////////////////////////////////////////////////////////////////
static void * ae_magic_memory_alloc_n( const aeMovieInstance * _instance, size_t _size, size_t _count )
{
	void * ptr = _instance->memory_alloc_n( _instance->instance_data, _size, _count );

	return ptr;
}
//////////////////////////////////////////////////////////////////////////
static void ae_magic_memory_free( const aeMovieInstance * _instance, const void * _ptr )
{
	_instance->memory_free( _instance->instance_data, _ptr );
}
//////////////////////////////////////////////////////////////////////////
static void ae_magic_memory_free_n( const aeMovieInstance * _instance, const void * _ptr )
{
	_instance->memory_free_n( _instance->instance_data, _ptr );
}
//////////////////////////////////////////////////////////////////////////
#	endif
//////////////////////////////////////////////////////////////////////////

#	define STRNCMP(instance, src, dst, count) (instance->strncmp(instance->instance_data, src, dst, count))

#	endif