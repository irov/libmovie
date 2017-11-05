/******************************************************************************
* libMOVIE Software License v1.0
*
* Copyright (c) 2016-2017, Yuriy Levchenko <irov13@mail.ru>
* All rights reserved.
*
* You are granted a perpetual, non-exclusive, non-sublicensable, and
* non-transferable license to use, install, execute, and perform the libMOVIE
* software and derivative works solely for personal or internal
* use. Without the written permission of Yuriy Levchenko, you may not (a) modify, translate,
* adapt, or develop new applications using the libMOVIE or otherwise
* create derivative works or improvements of the libMOVIE or (b) remove,
* delete, alter, or obscure any trademarks or any copyright, trademark, patent,
* or other intellectual property or proprietary rights notices on or in the
* Software, including any copy thereof. Redistributions in binary or source
* form must include this license and terms.
*
* THIS SOFTWARE IS PROVIDED BY YURIY LEVCHENKO "AS IS" AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL YURIY LEVCHENKO BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES, BUSINESS INTERRUPTION,
* OR LOSS OF USE, DATA, OR PROFITS) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#	include "movie/movie_instance.h"

#	include "movie_struct.h"

//////////////////////////////////////////////////////////////////////////
static ae_int32_t __ae_strncmp( ae_voidptr_t _data, const ae_char_t * _src, const ae_char_t * _dst, ae_size_t _count )
{
    (void)_data;

    for( ; _count > 0; _src++, _dst++, --_count )
    {
        if( *_src != *_dst )
        {
            if( *(ae_uint8_t *)_src < *(ae_uint8_t *)_dst )
            {
                return -1;
            }
            else
            {
                return +1;
            }
        }
        else if( *_src == '\0' )
        {
            return 0;
        }
    }

    return 0;
}
//////////////////////////////////////////////////////////////////////////
static void __ae_movie_logerror( ae_voidptr_t _data, aeMovieErrorCode _code, const ae_char_t * _message, ... )
{
    (void)_data;
    (void)_code;
    (void)_message;
    //SILENT
}
//////////////////////////////////////////////////////////////////////////
static void __instance_setup_bezier_warp( aeMovieInstance * _instance )
{
    ae_uint32_t i;
    for( i = 0; i != 10U; ++i )
    {
        ae_uint32_t line_count = AE_MOVIE_BEZIER_WARP_BASE_GRID + i * 2;

        ae_uint32_t vertex_count = line_count * line_count;        
        ae_vector2_t * bezier_warp_uv = _instance->memory_alloc_n( _instance->instance_data, sizeof( ae_vector2_t ), vertex_count );
                
        ae_vector2_t * bezier_warp_uv_iterator = bezier_warp_uv;
        
        ae_float_t grid_invf = 1.f / (ae_float_t)(line_count - 1);

        ae_uint32_t v = 0;
        for( ; v != line_count; ++v )
        {
            ae_uint32_t u = 0;
            for( ; u != line_count; ++u )
            {
                ae_vector2_t * uv = bezier_warp_uv_iterator++;
                (*uv)[0] = (ae_float_t)u * grid_invf;
                (*uv)[1] = (ae_float_t)v * grid_invf;
            }
        }

        _instance->bezier_warp_uv[i] = bezier_warp_uv;

        ae_uint32_t index_count = (line_count - 1) * (line_count - 1) * 6;
        ae_uint16_t * bezier_warp_indices = _instance->memory_alloc_n( _instance->instance_data, sizeof( ae_uint16_t ), index_count );

        ae_uint16_t * bezier_warp_indices_iterator = bezier_warp_indices;

        ae_uint32_t v2 = 0;
        for( ; v2 != line_count - 1; ++v2 )
        {
            ae_uint32_t u2 = 0;
            for( ; u2 != line_count - 1; ++u2 )
            {
                *bezier_warp_indices_iterator++ = u2 + (v2 + 0) * line_count + 0;
                *bezier_warp_indices_iterator++ = u2 + (v2 + 1) * line_count + 0;
                *bezier_warp_indices_iterator++ = u2 + (v2 + 0) * line_count + 1;
                *bezier_warp_indices_iterator++ = u2 + (v2 + 0) * line_count + 1;
                *bezier_warp_indices_iterator++ = u2 + (v2 + 1) * line_count + 0;
                *bezier_warp_indices_iterator++ = u2 + (v2 + 1) * line_count + 1;
            }
        }
        
        _instance->bezier_warp_indices[i] = bezier_warp_indices;
    }
}
//////////////////////////////////////////////////////////////////////////
aeMovieInstance * ae_create_movie_instance( const ae_char_t * _hashkey, ae_movie_alloc_t _alloc, ae_movie_alloc_n_t _alloc_n, ae_movie_free_t _free, ae_movie_free_n_t _free_n, ae_movie_strncmp_t _strncmp, ae_movie_logger_t _logger, ae_voidptr_t _data )
{
    aeMovieInstance * instance = (*_alloc)(_data, sizeof( aeMovieInstance ));

    instance->hashmask[0] = 0;
    instance->hashmask[1] = 0;
    instance->hashmask[2] = 0;
    instance->hashmask[3] = 0;
    instance->hashmask[4] = 0;

    ae_uint32_t i = 0;
    for( ; i != 41; ++i )
    {
        if( _hashkey[i] == '\0' && i != 40 )
        {
            return AE_NULL;
        }

        if( _hashkey[i] != '\0' && i == 40 )
        {
            return AE_NULL;
        }

        if( _hashkey[i] == '\0' && i == 40 )
        {
            break;
        }

        ae_uint32_t j = i / 8;
        ae_uint32_t k = i % 8;

        ae_char_t hash_char = _hashkey[i];

        ae_uint32_t v = (hash_char > '9') ? hash_char - 'a' + 10 : (hash_char - '0');

        instance->hashmask[j] += v << (k * 4);
    }

    instance->memory_alloc = _alloc;
    instance->memory_alloc_n = _alloc_n;
    instance->memory_free = _free;
    instance->memory_free_n = _free_n;
    instance->strncmp = _strncmp;
    instance->logger = _logger;
    instance->instance_data = _data;

    if( instance->strncmp == AE_NULL )
    {
        instance->strncmp = &__ae_strncmp;
    }

    if( instance->logger == AE_NULL )
    {
        instance->logger = &__ae_movie_logerror;
    }

    ae_float_t * sprite_uv = &instance->sprite_uv[0][0];

    *sprite_uv++ = 0.f;
    *sprite_uv++ = 0.f;
    *sprite_uv++ = 1.f;
    *sprite_uv++ = 0.f;
    *sprite_uv++ = 1.f;
    *sprite_uv++ = 1.f;
    *sprite_uv++ = 0.f;
    *sprite_uv++ = 1.f;

    ae_uint16_t * sprite_indices = instance->sprite_indices;

    *sprite_indices++ = 0;
    *sprite_indices++ = 3;
    *sprite_indices++ = 1;
    *sprite_indices++ = 1;
    *sprite_indices++ = 3;
    *sprite_indices++ = 2;

    __instance_setup_bezier_warp( instance );
    
    __clear_layer_extensions( &instance->layer_extensions_default );

    return instance;
}
//////////////////////////////////////////////////////////////////////////
void ae_delete_movie_instance( aeMovieInstance * _instance )
{
    (*_instance->memory_free)(_instance->instance_data, _instance);
}
//////////////////////////////////////////////////////////////////////////