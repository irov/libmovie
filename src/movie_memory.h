/******************************************************************************
* libMOVIE Software License v1.0
*
* Copyright (c) 2016-2017, Levchenko Yuriy <irov13@mail.ru>
* All rights reserved.
*
* You are granted a perpetual, non-exclusive, non-sublicensable, and
* non-transferable license to use, install, execute, and perform the libMOVIE
* software and derivative works solely for personal or internal
* use. Without the written permission of Levchenko Yuriy, you may not (a) modify, translate,
* adapt, or develop new applications using the libMOVIE or otherwise
* create derivative works or improvements of the libMOVIE or (b) remove,
* delete, alter, or obscure any trademarks or any copyright, trademark, patent,
* or other intellectual property or proprietary rights notices on or in the
* Software, including any copy thereof. Redistributions in binary or source
* form must include this license and terms.
*
* THIS SOFTWARE IS PROVIDED BY LEVCHENKO YURIY "AS IS" AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL LEVCHENKO YURIY BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES, BUSINESS INTERRUPTION,
* OR LOSS OF USE, DATA, OR PROFITS) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#	ifndef MOVIE_MEMORY_H_
#	define MOVIE_MEMORY_H_

#	include "movie/movie_type.h"

#	include "movie_struct.h"

//////////////////////////////////////////////////////////////////////////
#	ifdef AE_MOVIE_MEMORY_INFO
//////////////////////////////////////////////////////////////////////////
#	define NEW(instance, type) ((type *)ae_magic_memory_alloc_info( instance, #type, sizeof(type)))
#	define NEWV(instance, doc, size) ((void *)ae_magic_memory_alloc_info( instance, doc, size))
#	define NEWN(instance, type, n) (n == 0 ? (type *)AE_NULL : (type *)ae_magic_memory_alloc_n_info(instance, #type, sizeof(type), n))
#	define DELETE(instance, ptr) (ae_magic_memory_free_info(instance, #ptr, ptr))
#	define DELETEN(instance, ptr) (ae_magic_memory_free_n_info(instance, #ptr, ptr))
//////////////////////////////////////////////////////////////////////////
#	else
//////////////////////////////////////////////////////////////////////////
#	define NEW(instance, type) ((type *)ae_magic_memory_alloc( instance, sizeof(type)))
#	define NEWV(instance, doc, size) ((void *)ae_magic_memory_alloc( instance, size))
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
	_instance->logger( _instance->instance_data, AE_ERROR_MEMORY, "alloc type '%s' size '%d'\n", _type, (uint32_t)_size );

	void * ptr = _instance->memory_alloc( _instance->instance_data, _size );

	return ptr;
}
//////////////////////////////////////////////////////////////////////////
static void * ae_magic_memory_alloc_n_info( const aeMovieInstance * _instance, const char * _type, size_t _size, size_t _count )
{	
	_instance->logger( _instance->instance_data, AE_ERROR_MEMORY, "alloc n type '%s' size '%d' count '%d'\n", _type, (uint32_t)_size, (uint32_t)_count );

	void * ptr = _instance->memory_alloc_n( _instance->instance_data, _size, _count );

	return ptr;
}
//////////////////////////////////////////////////////////////////////////
static void ae_magic_memory_free_info( const aeMovieInstance * _instance, const char * _type, const void * _ptr )
{
	_instance->logger( _instance->instance_data, AE_ERROR_MEMORY, "free type '%s' ptr '%p'\n", _type, _ptr );

	_instance->memory_free( _instance->instance_data, _ptr );
}
//////////////////////////////////////////////////////////////////////////
static void ae_magic_memory_free_n_info( const aeMovieInstance * _instance, const char * _type, const void * _ptr )
{
	_instance->logger( _instance->instance_data, AE_ERROR_MEMORY, "free n type '%s' ptr '%p'\n", _type, _ptr );

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