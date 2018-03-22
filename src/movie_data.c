/******************************************************************************
* libMOVIE Software License v1.0
*
* Copyright (c) 2016-2018, Yuriy Levchenko <irov13@mail.ru>
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

#include "movie/movie_data.h"
#include "movie/movie_resource.h"

#include "movie_transformation.h"
#include "movie_memory.h"
#include "movie_stream.h"

//////////////////////////////////////////////////////////////////////////
aeMovieData * ae_create_movie_data( const aeMovieInstance * _instance, ae_movie_data_resource_provider_t _provider, ae_movie_data_resource_deleter_t _deleter, ae_voidptr_t _data )
{
    aeMovieData * movie = AE_NEW( _instance, aeMovieData );

    if( movie == AE_NULL )
    {
        return AE_NULL;
    }

    movie->instance = _instance;

    movie->name = AE_NULL;

    movie->resource_provider = _provider;
    movie->resource_deleter = _deleter;
    movie->resource_ud = _data;

    movie->resource_count = 0;
    movie->resources = AE_NULL;

    movie->composition_count = 0;
    movie->compositions = AE_NULL;

    return movie;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_void_t __ae_delete_mesh_t( const aeMovieInstance * _instance, const ae_mesh_t * _mesh )
{
    AE_DELETEN( _instance, _mesh->positions );
    AE_DELETEN( _instance, _mesh->uvs );
    AE_DELETEN( _instance, _mesh->indices );
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_void_t __ae_delete_layer_mesh_t( const aeMovieInstance * _instance, const aeMovieLayerMesh * _layerMesh, ae_uint32_t _count )
{
    if( _layerMesh->immutable == AE_TRUE )
    {
        __ae_delete_mesh_t( _instance, &_layerMesh->immutable_mesh );
    }
    else
    {
        const ae_mesh_t * it_mesh = _layerMesh->meshes;
        const ae_mesh_t * it_mesh_end = _layerMesh->meshes + _count;
        for( ; it_mesh != it_mesh_end; ++it_mesh )
        {
            const ae_mesh_t * mesh = it_mesh;

            __ae_delete_mesh_t( _instance, mesh );
        }

        AE_DELETEN( _instance, _layerMesh->meshes );
    }
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_void_t __ae_delete_property_value( const aeMovieInstance * _instance, const struct aeMoviePropertyValue * _property )
{
    AE_DELETEN( _instance, _property->values );
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_void_t __ae_delete_property_color( const aeMovieInstance * _instance, const struct aeMoviePropertyColor * _property )
{
    AE_DELETEN( _instance, _property->colors_r );
    AE_DELETEN( _instance, _property->colors_g );
    AE_DELETEN( _instance, _property->colors_b );
}
//////////////////////////////////////////////////////////////////////////
ae_void_t ae_delete_movie_data( const aeMovieData * _movieData )
{
    const aeMovieInstance * instance = _movieData->instance;

    const aeMovieResource * const * it_resource = _movieData->resources;
    const aeMovieResource * const * it_resource_end = _movieData->resources + _movieData->resource_count;
    for( ; it_resource != it_resource_end; ++it_resource )
    {
        const aeMovieResource * base_resource = *it_resource;

        aeMovieResourceTypeEnum type = base_resource->type;

        switch( type )
        {
        case AE_MOVIE_RESOURCE_NONE:
            {

            }break;
        case AE_MOVIE_RESOURCE_SOLID:
            {
                const aeMovieResourceSolid * resource = (const aeMovieResourceSolid *)base_resource;

                AE_UNUSED( resource );

                (*_movieData->resource_deleter)( type, base_resource->data, _movieData->resource_ud);

            }break;
        case AE_MOVIE_RESOURCE_VIDEO:
            {
                const aeMovieResourceVideo * resource = (const aeMovieResourceVideo *)base_resource;

                AE_DELETE_STRING( instance, resource->path );

                (*_movieData->resource_deleter)(type, base_resource->data, _movieData->resource_ud);

            }break;
        case AE_MOVIE_RESOURCE_SOUND:
            {
                const aeMovieResourceSound * resource = (const aeMovieResourceSound *)base_resource;

                AE_DELETE_STRING( instance, resource->path );

                (*_movieData->resource_deleter)(type, base_resource->data, _movieData->resource_ud);

            }break;
        case AE_MOVIE_RESOURCE_IMAGE:
            {
                const aeMovieResourceImage * resource = (const aeMovieResourceImage *)base_resource;

                AE_DELETE_STRING( instance, resource->path );

                if( resource->uv != instance->sprite_uv )
                {
                    AE_DELETEN( instance, resource->uv );
                }

                if( resource->mesh != AE_NULL )
                {
                    __ae_delete_mesh_t( instance, resource->mesh );

                    AE_DELETE( instance, resource->mesh );
                }

                (*_movieData->resource_deleter)(type, base_resource->data, _movieData->resource_ud);

            }break;
        case AE_MOVIE_RESOURCE_SEQUENCE:
            {
                const aeMovieResourceSequence * resource = (const aeMovieResourceSequence *)base_resource;

                AE_DELETEN( instance, resource->images );

                (*_movieData->resource_deleter)(type, base_resource->data, _movieData->resource_ud);

            }break;
        case AE_MOVIE_RESOURCE_PARTICLE:
            {
                const aeMovieResourceParticle * resource = (const aeMovieResourceParticle *)base_resource;

                AE_DELETE_STRING( instance, resource->path );

                (*_movieData->resource_deleter)(type, base_resource->data, _movieData->resource_ud);

            }break;
        case AE_MOVIE_RESOURCE_SLOT:
            {
                const aeMovieResourceSlot * resource = (const aeMovieResourceSlot *)base_resource;

                AE_UNUSED( resource );

                (*_movieData->resource_deleter)(type, base_resource->data, _movieData->resource_ud);

            }break;
        }

        AE_DELETE( instance, base_resource );
    }

    const aeMovieCompositionData * it_composition = _movieData->compositions;
    const aeMovieCompositionData * it_composition_end = _movieData->compositions + _movieData->composition_count;
    for( ; it_composition != it_composition_end; ++it_composition )
    {
        const aeMovieCompositionData * composition = it_composition;

        if( composition->camera != AE_NULL )
        {
            const aeMovieCompositionCamera * camera = composition->camera;

            AE_DELETE_STRING( instance, camera->name );

            AE_DELETE( instance, composition->camera );
        }

        const aeMovieLayerData * it_layer = composition->layers;
        const aeMovieLayerData * it_layer_end = composition->layers + composition->layer_count;
        for( ; it_layer != it_layer_end; ++it_layer )
        {
            const aeMovieLayerData * layer = it_layer;

            const aeMovieLayerExtensions * extensions = layer->extensions;

            if( extensions->timeremap != AE_NULL )
            {
                const aeMovieLayerTimeremap * timeremap = extensions->timeremap;

                AE_DELETEN( instance, timeremap->times );

                AE_DELETE( instance, extensions->timeremap );
            }

            if( extensions->mesh != AE_NULL )
            {
                const aeMovieLayerMesh * mesh = extensions->mesh;

                __ae_delete_layer_mesh_t( instance, mesh, layer->frame_count );

                AE_DELETE( instance, extensions->mesh );
            }

            if( extensions->bezier_warp != AE_NULL )
            {
                const aeMovieLayerBezierWarp * bezier_warp = extensions->bezier_warp;

                AE_DELETEN( instance, bezier_warp->bezier_warps );

                AE_DELETE( instance, extensions->bezier_warp );
            }

            if( extensions->color_vertex != AE_NULL )
            {
                const aeMovieLayerColorVertex * color_vertex = extensions->color_vertex;

                const struct aeMoviePropertyColor * property_color = color_vertex->property_color;

                __ae_delete_property_color( instance, property_color );

                AE_DELETE( instance, property_color );

                AE_DELETE( instance, extensions->color_vertex );
            }

            if( extensions->polygon != AE_NULL )
            {
                const aeMovieLayerPolygon * polygon = extensions->polygon;

                AE_DELETEN( instance, polygon->polygons );

                AE_DELETE( instance, extensions->polygon );
            }

            if( extensions->shader != AE_NULL )
            {
                const aeMovieLayerShader * shader = extensions->shader;

                AE_DELETE_STRING( instance, shader->name );
                AE_DELETE_STRING( instance, shader->shader_vertex );
                AE_DELETE_STRING( instance, shader->shader_fragment );

                const struct aeMovieLayerShaderParameter ** it_parameter = shader->parameters;
                const struct aeMovieLayerShaderParameter ** it_parameter_end = shader->parameters + shader->parameter_count;

                for( ;
                    it_parameter != it_parameter_end;
                    ++it_parameter )
                {
                    const struct aeMovieLayerShaderParameter * parameter = *it_parameter;

                    AE_DELETE_STRING( instance, parameter->name );

                    aeMovieShaderParameterTypeEnum parameter_type = parameter->type;

                    switch( parameter_type )
                    {
                    case AE_MOVIE_EXTENSION_SHADER_PARAMETER_SLIDER:
                        {
                            const struct aeMovieLayerShaderParameterSlider * parameter_slider = (const struct aeMovieLayerShaderParameterSlider *)parameter;

                            __ae_delete_property_value( instance, parameter_slider->property_value );

                            AE_DELETE( instance, parameter_slider->property_value );
                        }break;
                    case AE_MOVIE_EXTENSION_SHADER_PARAMETER_COLOR:
                        {
                            const struct aeMovieLayerShaderParameterColor * parameter_color = (const struct aeMovieLayerShaderParameterColor *)parameter;

                            __ae_delete_property_color( instance, parameter_color->property_color );

                            AE_DELETE( instance, parameter_color->property_color );
                        }break;
                    }
                }

                AE_DELETEN( instance, shader->parameters );

                AE_DELETE( instance, extensions->shader );
            }

            if( extensions->viewport != AE_NULL )
            {
                const aeMovieLayerViewport * viewport = extensions->viewport;

                AE_UNUSED( viewport );

                AE_DELETE( instance, extensions->viewport );
            }

            if( extensions != &instance->layer_extensions_default )
            {
                AE_DELETE( instance, layer->extensions );
            }

            ae_movie_delete_layer_transformation( instance, layer->transformation, layer->threeD );

            AE_DELETE( instance, layer->transformation );

            AE_DELETE_STRING( instance, layer->name );
        }

        AE_DELETEN( instance, composition->layers );

        AE_DELETE_STRING( instance, composition->name );
    }

    AE_DELETEN( instance, _movieData->resources );
    AE_DELETEN( instance, _movieData->compositions );

    if( _movieData->name != AE_NULL )
    {
        AE_DELETE_STRING( instance, _movieData->name );
    }

    AE_DELETE( instance, _movieData );
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_result_t __load_movie_data_composition_camera( aeMovieStream * _stream, aeMovieCompositionData * _compositionData )
{
    aeMovieCompositionCamera * camera = AE_NEW( _stream->instance, aeMovieCompositionCamera );

    AE_RESULT_PANIC_MEMORY( camera );

    ae_bool_t export_camera = AE_READB( _stream );

    if( export_camera == AE_FALSE )
    {
        ae_char_t * camera_name = AE_NEWN( _stream->instance, ae_char_t, 5 );

        AE_RESULT_PANIC_MEMORY( camera_name );

        camera_name[0] = 'N';
        camera_name[1] = 'o';
        camera_name[2] = 'n';
        camera_name[3] = 'e';
        camera_name[4] = '\0';

        camera->name = camera_name;
        
        AE_READF( _stream, camera->zoom );
        AE_READF( _stream, camera->fov );

        ae_float_t width = _compositionData->width;
        ae_float_t height = _compositionData->height;

        camera->immutable_property_mask = AE_MOVIE_IMMUTABLE_SUPER_CAMERA_ALL;

        camera->immutable.position_x = width * 0.5f;
        camera->immutable.position_y = height * 0.5f;
        camera->immutable.position_z = -camera->zoom;

        camera->immutable.target_x = width * 0.5f;
        camera->immutable.target_y = height * 0.5f;
        camera->immutable.target_z = 0.f;

        camera->immutable.quaternion_x = 0.f;
        camera->immutable.quaternion_y = 0.f;
        camera->immutable.quaternion_z = 0.f;
        camera->immutable.quaternion_w = 1.f;

        camera->timeline = AE_NULL;
    }
    else
    {
        AE_READ_STRING( _stream, camera->name );

        AE_READF( _stream, camera->zoom );
        AE_READF( _stream, camera->fov );

        AE_RESULT( ae_movie_load_camera_transformation, (_stream, camera) );
    }

    _compositionData->camera = camera;

    return AE_RESULT_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_bool_t __find_movie_data_composition_layer_position_by_index( const aeMovieCompositionData * _compositionData, const aeMovieLayerData * _layers, ae_uint32_t _index, ae_uint32_t * _position )
{
    ae_uint32_t iterator = 0U;

    const aeMovieLayerData * it_layer = _layers;
    const aeMovieLayerData * it_layer_end = _layers + _compositionData->layer_count;
    for( ; it_layer != it_layer_end; ++it_layer )
    {
        const aeMovieLayerData * layer = it_layer;

        if( layer->index == _index )
        {
            *_position = iterator;
            
            return AE_TRUE;
        }

        ++iterator;
    }

    return AE_FALSE;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_result_t __setup_movie_data_layer_track_matte( const aeMovieCompositionData * _compositionData, const aeMovieLayerData * _layers, aeMovieLayerData * _layer )
{
    if( _layer->has_track_matte == AE_TRUE )
    {
        ae_uint32_t layer_position;        
        if( __find_movie_data_composition_layer_position_by_index( _compositionData, _layers, _layer->index, &layer_position ) == AE_FALSE )
        {
            AE_RETURN_ERROR_RESULT( AE_RESULT_INVALID_DATA );
        }

        if( layer_position + 1 >= _compositionData->layer_count )
        {
            AE_RETURN_ERROR_RESULT( AE_RESULT_INVALID_DATA );
        }

        _layer->track_matte_layer = _compositionData->layers + layer_position + 1U;
    }
    else
    {
        _layer->track_matte_layer = AE_NULL;
    }

    return AE_RESULT_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_result_t __load_movie_property_color( aeMovieStream * _stream, struct aeMoviePropertyColor * _property, const aeMovieLayerData * _layer )
{
    _property->immutable_r = AE_READB( _stream );

    if( _property->immutable_r == AE_TRUE )
    {
        AE_READ_COLOR_CHANNEL( _stream, _property->immutable_color_r );

        _property->colors_r = AE_NULL;
    }
    else
    {
        _property->immutable_color_r = 1.f;

        ae_color_channel_t * colors_r = AE_NEWN( _stream->instance, ae_color_channel_t, _layer->frame_count );

        AE_RESULT_PANIC_MEMORY( colors_r );

        AE_READN( _stream, colors_r, _layer->frame_count );

        _property->colors_r = colors_r;
    }

    _property->immutable_g = AE_READB( _stream );

    if( _property->immutable_g == AE_TRUE )
    {
        AE_READ_COLOR_CHANNEL( _stream, _property->immutable_color_g );

        _property->colors_g = AE_NULL;
    }
    else
    {
        _property->immutable_color_g = 1.f;

        ae_color_channel_t * colors_g = AE_NEWN( _stream->instance, ae_color_channel_t, _layer->frame_count );
        
        AE_RESULT_PANIC_MEMORY( colors_g );

        AE_READN( _stream, colors_g, _layer->frame_count );

        _property->colors_g = colors_g;
    }

    _property->immutable_b = AE_READB( _stream );

    if( _property->immutable_b == AE_TRUE )
    {
        AE_READ_COLOR_CHANNEL( _stream, _property->immutable_color_b );

        _property->colors_b = AE_NULL;
    }
    else
    {
        _property->immutable_color_b = 1.f;

        ae_color_channel_t * colors_b = AE_NEWN( _stream->instance, ae_color_channel_t, _layer->frame_count );

        AE_RESULT_PANIC_MEMORY( colors_b );

        AE_READN( _stream, colors_b, _layer->frame_count );

        _property->colors_b = colors_b;
    }

    return AE_RESULT_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_result_t __load_movie_property_value( aeMovieStream * _stream, struct aeMoviePropertyValue * _property, const aeMovieLayerData * _layer )
{
    _property->immutable = AE_READB( _stream );

    if( _property->immutable == AE_TRUE )
    {
        AE_READF( _stream, _property->immutable_value );

        _property->values = AE_NULL;
    }
    else
    {
        _property->immutable_value = 0.f;

        ae_float_t * values = AE_NEWN( _stream->instance, ae_float_t, _layer->frame_count );
        
        AE_RESULT_PANIC_MEMORY( values );

        AE_READN( _stream, values, _layer->frame_count );

        _property->values = values;
    }

    return AE_RESULT_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL aeMovieLayerExtensions * __request_extensions( const aeMovieInstance * _instance, aeMovieLayerExtensions * _extensions )
{
    if( _extensions != AE_NULL )
    {
        return _extensions;
    }
     
    aeMovieLayerExtensions * extensions = AE_NEW( _instance, aeMovieLayerExtensions );

    if( extensions == AE_NULL )
    {
        return AE_NULL;
    }

    __clear_layer_extensions( extensions );

    return extensions;    
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_result_t __load_movie_data_layer( const aeMovieData * _movieData, const aeMovieCompositionData * _compositions, aeMovieStream * _stream, const aeMovieCompositionData * _compositionData, aeMovieLayerData * _layer )
{
    const aeMovieInstance * instance = _movieData->instance;

    AE_READ_STRING( _stream, _layer->name );

    _layer->index = AE_READZ( _stream );

    _layer->is_track_matte = AE_READB( _stream );
    _layer->has_track_matte = AE_READB( _stream );

    if( _layer->has_track_matte == AE_TRUE )
    {
        _layer->track_matte_mode = AE_READ8( _stream );
    }
    else
    {
        _layer->track_matte_mode = AE_MOVIE_TRACK_MATTE_NONE;
    }

    ae_uint8_t type;
    AE_READ( _stream, type );
    _layer->type = type;

    _layer->frame_count = AE_READZ( _stream );

    aeMovieLayerExtensions * layer_extensions = AE_NULL;

    for( ;; )
    {
        ae_uint8_t extension;
        AE_READ( _stream, extension );

        switch( extension )
        {
        case 0:
            {
            }break;
        case AE_LAYER_EXTENSION_TIMEREMAP:
            {
                aeMovieLayerTimeremap * layer_timeremap = AE_NEW( instance, aeMovieLayerTimeremap );

                AE_RESULT_PANIC_MEMORY( layer_timeremap );

                ae_float_t * times = AE_NEWN( instance, ae_float_t, _layer->frame_count );

                AE_RESULT_PANIC_MEMORY( times );

                AE_READN( _stream, times, _layer->frame_count );

                layer_timeremap->times = times;

                for( ;; )
                {
                    ae_uint8_t params;
                    AE_READ( _stream, params );

                    switch( params )
                    {
                    case 0:
                        {
                        }break;
                    default:
                        {
                            AE_RETURN_ERROR_RESULT( AE_RESULT_INVALID_STREAM );
                        }break;
                    }

                    if( params == 0 )
                    {
                        break;
                    }
                }

                layer_extensions = __request_extensions( instance, layer_extensions );

                AE_RESULT_PANIC_MEMORY( layer_extensions );

                layer_extensions->timeremap = layer_timeremap;
            }break;
        case AE_LAYER_EXTENSION_MESH:
            {
                aeMovieLayerMesh * layer_mesh = AE_NEW( instance, aeMovieLayerMesh );

                AE_RESULT_PANIC_MEMORY( layer_mesh );

                layer_mesh->immutable = AE_READB( _stream );

                if( layer_mesh->immutable == AE_TRUE )
                {
                    AE_READ_MESH( _stream, &layer_mesh->immutable_mesh );

                    layer_mesh->meshes = AE_NULL;
                }
                else
                {
                    ae_mesh_t * meshes = AE_NEWN( instance, ae_mesh_t, _layer->frame_count );

                    AE_RESULT_PANIC_MEMORY( meshes );

                    ae_mesh_t * it_mesh = meshes;
                    ae_mesh_t * it_mesh_end = meshes + _layer->frame_count;
                    for( ; it_mesh != it_mesh_end; ++it_mesh )
                    {
                        AE_READ_MESH( _stream, it_mesh );
                    }

                    layer_mesh->meshes = meshes;
                }

                for( ;; )
                {
                    ae_uint8_t params;
                    AE_READ( _stream, params );

                    switch( params )
                    {
                    case 0:
                        {
                        }break;
                    default:
                        {
                            AE_RETURN_ERROR_RESULT( AE_RESULT_INVALID_STREAM );
                        }break;
                    }

                    if( params == 0 )
                    {
                        break;
                    }
                }

                layer_extensions = __request_extensions( instance, layer_extensions );

                AE_RESULT_PANIC_MEMORY( layer_extensions );

                layer_extensions->mesh = layer_mesh;
            }break;
        case AE_LAYER_EXTENSION_BEZIERWARP:
            {
                aeMovieLayerBezierWarp * layer_bezier_warp = AE_NEW( instance, aeMovieLayerBezierWarp );

                AE_RESULT_PANIC_MEMORY( layer_bezier_warp );

                layer_bezier_warp->immutable = AE_READB( _stream );

                if( layer_bezier_warp->immutable == AE_TRUE )
                {
                    AE_READN( _stream, layer_bezier_warp->immutable_bezier_warp.corners, 4 );
                    AE_READN( _stream, layer_bezier_warp->immutable_bezier_warp.beziers, 8 );

                    layer_bezier_warp->bezier_warps = AE_NULL;
                }
                else
                {
                    aeMovieBezierWarp * bezier_warps = AE_NEWN( instance, aeMovieBezierWarp, _layer->frame_count );

                    AE_RESULT_PANIC_MEMORY( bezier_warps );

                    aeMovieBezierWarp * it_bezier_warp = bezier_warps;
                    aeMovieBezierWarp * it_bezier_warp_end = bezier_warps + _layer->frame_count;
                    for( ; it_bezier_warp != it_bezier_warp_end; ++it_bezier_warp )
                    {
                        AE_READN( _stream, it_bezier_warp->corners, 4 );
                        AE_READN( _stream, it_bezier_warp->beziers, 8 );
                    }

                    layer_bezier_warp->bezier_warps = bezier_warps;
                }

                ae_uint8_t quality;
                AE_READ( _stream, quality );

                layer_bezier_warp->quality = quality;

                for( ;; )
                {
                    ae_uint8_t params;
                    AE_READ( _stream, params );

                    switch( params )
                    {
                    case 0:
                        {
                        }break;
                    default:
                        {
                            AE_RETURN_ERROR_RESULT( AE_RESULT_INVALID_STREAM );
                        }break;
                    }

                    if( params == 0 )
                    {
                        break;
                    }
                }

                layer_extensions = __request_extensions( instance, layer_extensions );

                AE_RESULT_PANIC_MEMORY( layer_extensions );

                layer_extensions->bezier_warp = layer_bezier_warp;
            }break;
        case AE_LAYER_EXTENSION_COLORVERTEX:
            {
                aeMovieLayerColorVertex * layer_color_vertex = AE_NEW( instance, aeMovieLayerColorVertex );

                AE_RESULT_PANIC_MEMORY( layer_color_vertex );

                struct aeMoviePropertyColor * property_color = AE_NEW( instance, struct aeMoviePropertyColor );

                AE_RESULT_PANIC_MEMORY( property_color );

                AE_RESULT( __load_movie_property_color, (_stream, property_color, _layer) );

                layer_color_vertex->property_color = property_color;

                for( ;; )
                {
                    ae_uint8_t params;
                    AE_READ( _stream, params );

                    switch( params )
                    {
                    case 0:
                        {
                        }break;
                    default:
                        {
                            AE_RETURN_ERROR_RESULT( AE_RESULT_INVALID_STREAM );
                        }break;
                    }

                    if( params == 0 )
                    {
                        break;
                    }
                }

                layer_extensions = __request_extensions( instance, layer_extensions );

                AE_RESULT_PANIC_MEMORY( layer_extensions );

                layer_extensions->color_vertex = layer_color_vertex;
            }break;
        case AE_LAYER_EXTENSION_POLYGON:
            {
                aeMovieLayerPolygon * layer_polygon = AE_NEW( instance, aeMovieLayerPolygon );

                AE_RESULT_PANIC_MEMORY( layer_polygon );

                layer_polygon->immutable = AE_READB( _stream );

                if( layer_polygon->immutable == AE_TRUE )
                {
                    AE_READ_POLYGON( _stream, &layer_polygon->immutable_polygon );

                    layer_polygon->polygons = AE_NULL;
                }
                else
                {
                    ae_uint32_t polygon_count = AE_READZ( _stream );

                    ae_polygon_t * polygons = AE_NEWN( instance, ae_polygon_t, polygon_count );

                    AE_RESULT_PANIC_MEMORY( polygons );

                    ae_polygon_t * it_polygon = polygons;
                    ae_polygon_t * it_polygon_end = polygons + polygon_count;
                    for( ; it_polygon != it_polygon_end; ++it_polygon )
                    {
                        AE_READ_POLYGON( _stream, it_polygon );
                    }

                    layer_polygon->polygons = polygons;
                }

                for( ;; )
                {
                    ae_uint8_t params;
                    AE_READ( _stream, params );

                    switch( params )
                    {
                    case 0:
                        {
                        }break;
                    default:
                        {
                            AE_RETURN_ERROR_RESULT( AE_RESULT_INVALID_STREAM );
                        }break;
                    }

                    if( params == 0 )
                    {
                        break;
                    }
                }
                
                layer_extensions = __request_extensions( instance, layer_extensions );

                AE_RESULT_PANIC_MEMORY( layer_extensions );

                layer_extensions->polygon = layer_polygon;
            }break;
        case AE_LAYER_EXTENSION_SHADER:
            {
                aeMovieLayerShader * layer_shader = AE_NEW( instance, aeMovieLayerShader );

                AE_RESULT_PANIC_MEMORY( layer_shader );

                AE_READ_STRING( _stream, layer_shader->name );
                AE_READ( _stream, layer_shader->version );

                AE_READ_STRING( _stream, layer_shader->shader_vertex );
                AE_READ_STRING( _stream, layer_shader->shader_fragment );

                layer_shader->parameter_count = AE_READZ( _stream );
                
                const struct aeMovieLayerShaderParameter ** parameters = AE_NEWN( _stream->instance, const struct aeMovieLayerShaderParameter *, layer_shader->parameter_count );

                AE_RESULT_PANIC_MEMORY( parameters );

                const struct aeMovieLayerShaderParameter ** it_parameter = parameters;
                const struct aeMovieLayerShaderParameter ** it_parameter_end = parameters + layer_shader->parameter_count;

                for( ;
                    it_parameter != it_parameter_end;
                    ++it_parameter )
                {
                    ae_uint8_t paramater_type;
                    AE_READ( _stream, paramater_type );

                    switch( paramater_type )
                    {
                    case AE_MOVIE_EXTENSION_SHADER_PARAMETER_SLIDER:
                        {
                            struct aeMovieLayerShaderParameterSlider * parameter_slider = AE_NEW( _stream->instance, struct aeMovieLayerShaderParameterSlider );

                            AE_RESULT_PANIC_MEMORY( parameter_slider );

                            parameter_slider->type = paramater_type;
                            AE_READ_STRING( _stream, parameter_slider->name );                                                        
                            AE_READ_STRING( _stream, parameter_slider->uniform );

                            struct aeMoviePropertyValue * property_value = AE_NEW( _stream->instance, struct aeMoviePropertyValue );

                            AE_RESULT_PANIC_MEMORY( property_value );

                            AE_RESULT( __load_movie_property_value, (_stream, property_value, _layer) );

                            parameter_slider->property_value = property_value;

                            *it_parameter = (struct aeMovieLayerShaderParameter *)parameter_slider;
                        }break;
                    case AE_MOVIE_EXTENSION_SHADER_PARAMETER_COLOR:
                        {
                            struct aeMovieLayerShaderParameterColor * parameter_color = AE_NEW( _stream->instance, struct aeMovieLayerShaderParameterColor );

                            AE_RESULT_PANIC_MEMORY( parameter_color );

                            parameter_color->type = paramater_type;
                            AE_READ_STRING( _stream, parameter_color->name );
                            AE_READ_STRING( _stream, parameter_color->uniform );

                            struct aeMoviePropertyColor * property_color = AE_NEW( _stream->instance, struct aeMoviePropertyColor );

                            AE_RESULT_PANIC_MEMORY( property_color );

                            AE_RESULT( __load_movie_property_color, (_stream, property_color, _layer) );

                            parameter_color->property_color = property_color;

                            *it_parameter = (struct aeMovieLayerShaderParameter *)parameter_color;
                        }break;
                    }
                }

                layer_shader->parameters = parameters;

                for( ;; )
                {
                    ae_uint8_t params;
                    AE_READ( _stream, params );

                    switch( params )
                    {
                    case 0:
                        {
                        }break;
                    default:
                        {
                            AE_RETURN_ERROR_RESULT( AE_RESULT_INVALID_STREAM );
                        }break;
                    }

                    if( params == 0 )
                    {
                        break;
                    }
                }

                layer_extensions = __request_extensions( instance, layer_extensions );

                AE_RESULT_PANIC_MEMORY( layer_extensions );

                layer_extensions->shader = layer_shader;
            }break;
        case AE_LAYER_EXTENSION_VIEWPORT:
            {
                aeMovieLayerViewport * layer_viewport = AE_NEW( instance, aeMovieLayerViewport );

                AE_RESULT_PANIC_MEMORY( layer_viewport );

                ae_magic_read_viewport( _stream, &layer_viewport->viewport );

                for( ;; )
                {
                    ae_uint8_t params;
                    AE_READ( _stream, params );

                    switch( params )
                    {
                    case 0:
                        {
                        }break;
                    default:
                        {
                            AE_RETURN_ERROR_RESULT( AE_RESULT_INVALID_STREAM );
                        }break;
                    }

                    if( params == 0 )
                    {
                        break;
                    }
                }

                layer_extensions = __request_extensions( instance, layer_extensions );

                AE_RESULT_PANIC_MEMORY( layer_extensions );

                layer_extensions->viewport = layer_viewport;
            }break;
        default:
            {
                AE_RETURN_ERROR_RESULT( AE_RESULT_INVALID_STREAM );
            }break;
        }

        if( extension == 0 )
        {
            break;
        }
    }

    if( layer_extensions == AE_NULL )
    {
        _layer->extensions = &instance->layer_extensions_default;
    }
    else
    {
        _layer->extensions = layer_extensions;
    }

    ae_bool_t is_resource_or_composition = AE_READB( _stream );

    if( is_resource_or_composition == AE_TRUE )
    {
        ae_uint32_t resource_index = AE_READZ( _stream );

        if( resource_index == 0 )
        {
            _layer->resource = AE_NULL;
        }
        else
        {
            _layer->resource = _movieData->resources[resource_index - 1];
        }

        _layer->sub_composition_data = AE_NULL;
    }
    else
    {
        ae_uint32_t composition_index = AE_READZ( _stream );
        _layer->sub_composition_data = _compositions + composition_index;

        _layer->resource = AE_NULL;
    }

    ae_uint32_t parent_index = AE_READZ( _stream );

    _layer->parent_index = parent_index;

    _layer->reverse_time = AE_READB( _stream );

    AE_READF( _stream, _layer->start_time );
    AE_READF( _stream, _layer->in_time );
    AE_READF( _stream, _layer->out_time );

    if( _layer->out_time > _compositionData->duration )
    {
        _layer->trimmed_time = AE_TRUE;
        _layer->out_time = _compositionData->duration;
    }
    else
    {
        _layer->trimmed_time = AE_FALSE;
    }

    ae_uint8_t blend_mode;
    AE_READ( _stream, blend_mode );
    _layer->blend_mode = blend_mode;

    _layer->threeD = AE_READB( _stream );
    AE_READ( _stream, _layer->params );

    _layer->play_count = AE_READZ( _stream );

    AE_READF( _stream, _layer->stretch );

    aeMovieLayerTransformation * transformation = AE_NULL;

    if( _layer->threeD == AE_FALSE )
    {
        transformation = (aeMovieLayerTransformation *)AE_NEW( instance, aeMovieLayerTransformation2D );

        AE_RESULT_PANIC_MEMORY( transformation );
    }
    else
    {
        transformation = (aeMovieLayerTransformation *)AE_NEW( instance, aeMovieLayerTransformation3D );

        AE_RESULT_PANIC_MEMORY( transformation );
    }

    AE_RESULT( ae_movie_load_layer_transformation, (_stream, transformation, _layer->threeD) );

    _layer->transformation = transformation;

    if( _layer->is_track_matte == AE_TRUE )
    {
        _layer->renderable = AE_FALSE;
    }
    else
    {
        aeMovieLayerTypeEnum layer_type = _layer->type;

        switch( layer_type )
        {
        case AE_MOVIE_LAYER_TYPE_MOVIE:
            {
                _layer->renderable = AE_FALSE;
            }break;
        case AE_MOVIE_LAYER_TYPE_TEXT:
            {
                _layer->renderable = AE_FALSE;
            }break;
        case AE_MOVIE_LAYER_TYPE_EVENT:
            {
                _layer->renderable = AE_FALSE;
            }break;
        case AE_MOVIE_LAYER_TYPE_SOCKET:
            {
                _layer->renderable = AE_FALSE;
            }break;
        case AE_MOVIE_LAYER_TYPE_SHAPE:
            {
                _layer->renderable = AE_TRUE;
            }break;
        case AE_MOVIE_LAYER_TYPE_SLOT:
            {
                _layer->renderable = AE_TRUE;
            }break;
        case AE_MOVIE_LAYER_TYPE_NULL:
            {
                _layer->renderable = AE_FALSE;
            }break;
        case AE_MOVIE_LAYER_TYPE_SOLID:
            {
                _layer->renderable = AE_TRUE;
            }break;
        case AE_MOVIE_LAYER_TYPE_SEQUENCE:
            {
                _layer->renderable = AE_TRUE;
            }break;
        case AE_MOVIE_LAYER_TYPE_VIDEO:
            {
                _layer->renderable = AE_TRUE;
            }break;
        case AE_MOVIE_LAYER_TYPE_SOUND:
            {
                _layer->renderable = AE_FALSE;
            }break;
        case AE_MOVIE_LAYER_TYPE_PARTICLE:
            {
                _layer->renderable = AE_TRUE;
            }break;
        case AE_MOVIE_LAYER_TYPE_IMAGE:
            {
                _layer->renderable = AE_TRUE;
            }break;
        case AE_MOVIE_LAYER_TYPE_SUB_MOVIE:
            {
                _layer->renderable = AE_FALSE;
            }break;
        default:
            {
                AE_RETURN_ERROR_RESULT( AE_RESULT_INVALID_DATA );
            }break;
        }
    }

    return AE_RESULT_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_result_t __load_movie_data_composition_layers( const aeMovieData * _movieData, const aeMovieCompositionData * _compositions, aeMovieLayerData * _layers, aeMovieStream * _stream, aeMovieCompositionData * _compositionData )
{
    aeMovieLayerData * it_layer = _layers;
    aeMovieLayerData * it_layer_end = _layers + _compositionData->layer_count;
    for( ; it_layer != it_layer_end; ++it_layer )
    {
        aeMovieLayerData * layer = it_layer;

        layer->composition_data = _compositionData;

        AE_RESULT( __load_movie_data_layer, (_movieData, _compositions, _stream, _compositionData, layer) );
    }

    return AE_RESULT_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_result_t __setup_movie_data_composition_layers( const aeMovieCompositionData * _compositionData, aeMovieLayerData * _layers )
{
    aeMovieLayerData * it_layer = _layers;
    aeMovieLayerData * it_layer_end = _layers + _compositionData->layer_count;
    for( ; it_layer != it_layer_end; ++it_layer )
    {
        aeMovieLayerData * layer = it_layer;

        AE_RESULT( __setup_movie_data_layer_track_matte, (_compositionData, _layers, layer) );
    }

    return AE_RESULT_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_result_t __load_movie_data_composition( const aeMovieData * _movieData, const aeMovieCompositionData * _compositions, aeMovieStream * _stream, aeMovieCompositionData * _compositionData )
{
    const aeMovieInstance * instance = _movieData->instance;

    AE_READ_STRING( _stream, _compositionData->name );

    _compositionData->master = AE_READB( _stream );

    AE_READF( _stream, _compositionData->width );
    AE_READF( _stream, _compositionData->height );

    AE_READF( _stream, _compositionData->duration );
    AE_READF( _stream, _compositionData->frameDuration );
    AE_READF( _stream, _compositionData->frameDurationInv );

    _compositionData->camera = AE_NULL;

    _compositionData->frameCount = (ae_uint32_t)(_compositionData->duration * _compositionData->frameDurationInv + 0.5f);

    _compositionData->flags = 0;

    _compositionData->loop_segment[0] = 0.f;
    _compositionData->loop_segment[1] = _compositionData->duration;

    _compositionData->anchor_point[0] = 0.f;
    _compositionData->anchor_point[1] = 0.f;
    _compositionData->anchor_point[2] = 0.f;

    _compositionData->offset_point[0] = 0.f;
    _compositionData->offset_point[1] = 0.f;
    _compositionData->offset_point[2] = 0.f;

    _compositionData->bounds[0] = 0.f;
    _compositionData->bounds[1] = 0.f;
    _compositionData->bounds[2] = 0.f;
    _compositionData->bounds[3] = 0.f;

    _compositionData->camera = AE_NULL;

    for( ;; )
    {
        ae_uint8_t flag;
        AE_READ( _stream, flag );

        switch( flag )
        {
        case 0:
            {
            }break;
        case 1:
            {
                AE_READF2( _stream, _compositionData->loop_segment );

                _compositionData->flags |= AE_MOVIE_COMPOSITION_LOOP_SEGMENT;
            }break;
        case 2:
            {
                AE_READF3( _stream, _compositionData->anchor_point );

                _compositionData->flags |= AE_MOVIE_COMPOSITION_ANCHOR_POINT;
            }break;
        case 3:
            {
                AE_READF3( _stream, _compositionData->offset_point );

                _compositionData->flags |= AE_MOVIE_COMPOSITION_OFFSET_POINT;
            }break;
        case 4:
            {
                AE_READF4( _stream, _compositionData->bounds );

                _compositionData->flags |= AE_MOVIE_COMPOSITION_BOUNDS;
            }break;
        case 5:
            {
                AE_RESULT( __load_movie_data_composition_camera, ( _stream, _compositionData ) );

                _compositionData->flags |= AE_MOVIE_COMPOSITION_CAMERA;
            }break;
        default:
            {
                AE_RETURN_ERROR_RESULT( AE_RESULT_INVALID_STREAM );
            }break;
        };

        if( flag == 0 )
        {
            break;
        }
    }

    ae_uint32_t layer_count = AE_READZ( _stream );

    _compositionData->layer_count = layer_count;
    aeMovieLayerData * layers = AE_NEWN( instance, aeMovieLayerData, layer_count );

    AE_RESULT_PANIC_MEMORY( layers );

    AE_RESULT( __load_movie_data_composition_layers, (_movieData, _compositions, layers, _stream, _compositionData) );

    AE_RESULT( __setup_movie_data_composition_layers, (_compositionData, layers) );   

    _compositionData->layers = layers;

    return AE_RESULT_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
aeMovieStream * ae_create_movie_stream( const aeMovieInstance * _instance, ae_movie_stream_memory_read_t _read, ae_movie_stream_memory_copy_t _copy, ae_voidptr_t _data )
{
#ifdef AE_MOVIE_DEBUG
    if( _instance == AE_NULL )
    {
        return AE_NULL;
    }

    if( _read == AE_NULL )
    {
        return AE_NULL;
    }

    if( _copy == AE_NULL )
    {
        return AE_NULL;
    }
#endif

    aeMovieStream * stream = AE_NEW( _instance, aeMovieStream );

    AE_MOVIE_PANIC_MEMORY( stream, AE_NULL );

    stream->instance = _instance;
    stream->memory_read = _read;
    stream->memory_copy = _copy;
    stream->read_data = _data;
    stream->copy_data = _data;

    stream->buffer = AE_NULL;
    stream->carriage = 0U;

    return stream;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_size_t __ae_read_buffer( ae_voidptr_t _data, ae_voidptr_t _buff, ae_size_t _carriage, ae_size_t _size )
{
    aeMovieStream * stream = (aeMovieStream *)_data;

    stream->memory_copy( stream->copy_data, (ae_constbyteptr_t)stream->buffer + _carriage, _buff, _size );
        
    return _size;
}
//////////////////////////////////////////////////////////////////////////
aeMovieStream * ae_create_movie_stream_memory( const aeMovieInstance * _instance, ae_constvoidptr_t _buffer, ae_movie_stream_memory_copy_t _copy, ae_voidptr_t _data )
{
#ifdef AE_MOVIE_DEBUG
    if( _instance == AE_NULL )
    {
        return AE_NULL;
    }

    if( _copy == AE_NULL )
    {
        return AE_NULL;
    }
#endif

    aeMovieStream * stream = AE_NEW( _instance, aeMovieStream );

    AE_MOVIE_PANIC_MEMORY( stream, AE_NULL );

    stream->instance = _instance;
    stream->memory_read = &__ae_read_buffer;
    stream->memory_copy = _copy;
    stream->read_data = stream;
    stream->copy_data = _data;

    stream->buffer = _buffer;
    stream->carriage = 0U;

    return stream;
}
//////////////////////////////////////////////////////////////////////////
ae_void_t ae_delete_movie_stream( aeMovieStream * _stream )
{
    AE_DELETE( _stream->instance, _stream );
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_result_t __check_movie_data( aeMovieStream * _stream, ae_uint32_t * _major, ae_uint32_t * _minor )
{
    ae_uint8_t magic[4];
    AE_READN( _stream, magic, 4 );

    if( magic[0] != 'A' ||
        magic[1] != 'E' ||
        magic[2] != 'M' ||
        magic[3] != '1' )
    {
        AE_RETURN_ERROR_RESULT( AE_RESULT_INVALID_MAGIC );
    }

    ae_uint32_t major_version;
    AE_READ( _stream, major_version );

    ae_uint32_t minor_version = AE_MOVIE_SDK_MINOR_VERSION;

    *_major = major_version;
    *_minor = minor_version;

    if( major_version != AE_MOVIE_SDK_MAJOR_VERSION )
    {
        AE_RETURN_ERROR_RESULT( AE_RESULT_INVALID_VERSION );
    }

    if( minor_version != AE_MOVIE_SDK_MINOR_VERSION )
    {
        AE_RETURN_ERROR_RESULT( AE_RESULT_INVALID_VERSION );
    }

    ae_uint32_t hash_crc;
    AE_READ( _stream, hash_crc );

    ae_uint32_t ae_movie_hashmask_crc =
        _stream->instance->hashmask[0] ^
        _stream->instance->hashmask[1] ^
        _stream->instance->hashmask[2] ^
        _stream->instance->hashmask[3] ^
        _stream->instance->hashmask[4];

    if( hash_crc != ae_movie_hashmask_crc )
    {
        AE_RETURN_ERROR_RESULT( AE_RESULT_INVALID_HASH );
    }

    return AE_RESULT_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ae_result_t ae_check_movie_data( aeMovieStream * _stream, ae_uint32_t * _major, ae_uint32_t * _minor )
{
    ae_result_t result = __check_movie_data( _stream, _major, _minor );
    
    return result;
}
//////////////////////////////////////////////////////////////////////////
ae_uint32_t ae_get_movie_sdk_major_version( ae_void_t )
{
    return AE_MOVIE_SDK_MAJOR_VERSION;
}
//////////////////////////////////////////////////////////////////////////
ae_uint32_t ae_get_movie_sdk_minor_version( ae_void_t )
{
    return AE_MOVIE_SDK_MINOR_VERSION;
}
//////////////////////////////////////////////////////////////////////////
const ae_char_t * ae_get_result_string_info( ae_result_t _result )
{
    switch( _result )
    {
    case AE_RESULT_SUCCESSFUL:
        {
            return "successful";
        }break;
    case AE_RESULT_INVALID_MAGIC:
        {
            return "invalid magic number";
        }break;
    case AE_RESULT_INVALID_VERSION:
        {
            return "invalid version";
        }break;
    case AE_RESULT_INVALID_HASH:
        {
            return "invalid hash";
        }break;
    case AE_RESULT_INVALID_STREAM:
        {
            return "invalid stream";
        }break;
    case AE_RESULT_INVALID_DATA:
        {
            return "invalid data";
        }break;
    case AE_RESULT_INVALID_MEMORY:
        {
            return "invalid memory";
        }break;
    }

    return "invalid result";
}
//////////////////////////////////////////////////////////////////////////
ae_result_t ae_load_movie_data( aeMovieData * _movieData, aeMovieStream * _stream, ae_uint32_t * _major, ae_uint32_t * _minor )
{
    const aeMovieInstance * instance = _movieData->instance;

#ifdef AE_MOVIE_DEBUG_STREAM
    instance->logger( instance->instance_data, AE_ERROR_STREAM, "begin" );
#endif

    ae_result_t check_result = __check_movie_data( _stream, _major, _minor );

    if( check_result != AE_RESULT_SUCCESSFUL )
    {
        return check_result;
    }
    
    AE_READ_STRING( _stream, _movieData->name );

    ae_uint32_t resource_count = AE_READZ( _stream );

    _movieData->resource_count = resource_count;
    const aeMovieResource ** resources = AE_NEWN( instance, const aeMovieResource *, resource_count );

    AE_RESULT_PANIC_MEMORY( resources );

    const aeMovieResource ** it_resource = resources;
    const aeMovieResource ** it_resource_end = resources + resource_count;
    for( ; it_resource != it_resource_end; ++it_resource )
    {
        ae_uint8_t type;
        AE_READ( _stream, type );

#ifdef AE_MOVIE_DEBUG_STREAM
        instance->logger( instance->instance_data, AE_ERROR_STREAM, "read type %d", type );
#endif

        switch( type )
        {
        case AE_MOVIE_RESOURCE_SOLID:
            {
                aeMovieResourceSolid * resource = AE_NEW( instance, aeMovieResourceSolid );

                AE_RESULT_PANIC_MEMORY( resource );

                AE_READF( _stream, resource->width );
                AE_READF( _stream, resource->height );
                AE_READ_COLOR( _stream, &resource->color );

                for( ;;)
                {
                    ae_uint8_t param_type;
                    AE_READ( _stream, param_type );

                    switch( param_type )
                    {
                    case 0:
                        {
                        }break;
                    default:
                        {
                            AE_RETURN_ERROR_RESULT( AE_RESULT_INVALID_STREAM );
                        }
                    }

                    if( param_type == 0 )
                    {
                        break;
                    }
                }

                *it_resource = (aeMovieResource *)resource;

                resource->type = type;
                resource->data = (*_movieData->resource_provider)(*it_resource, _movieData->resource_ud);
            }break;
        case AE_MOVIE_RESOURCE_VIDEO:
            {
                aeMovieResourceVideo * resource = AE_NEW( instance, aeMovieResourceVideo );

                AE_RESULT_PANIC_MEMORY( resource );

                AE_READ_STRING( _stream, resource->path );
                resource->codec = AE_READ8( _stream );

                AE_READF( _stream, resource->width );
                AE_READF( _stream, resource->height );

                ae_uint8_t alpha;
                AE_READ( _stream, alpha );
                resource->alpha = alpha;

                AE_READF( _stream, resource->frameRate );
                AE_READF( _stream, resource->duration );

                for( ;;)
                {
                    ae_uint8_t param_type;
                    AE_READ( _stream, param_type );

                    switch( param_type )
                    {
                    case 0:
                        {
                        }break;
                    default:
                        {
                            AE_RETURN_ERROR_RESULT( AE_RESULT_INVALID_STREAM );
                        }
                    }

                    if( param_type == 0 )
                    {
                        break;
                    }
                }

                *it_resource = (aeMovieResource *)resource;

                resource->type = type;
                resource->data = (*_movieData->resource_provider)(*it_resource, _movieData->resource_ud);
            }break;
        case AE_MOVIE_RESOURCE_SOUND:
            {
                aeMovieResourceSound * resource = AE_NEW( instance, aeMovieResourceSound );

                AE_RESULT_PANIC_MEMORY( resource );

                AE_READ_STRING( _stream, resource->path );
                resource->codec = AE_READ8( _stream );

                AE_READF( _stream, resource->duration );

                for( ;;)
                {
                    ae_uint8_t param_type;
                    AE_READ( _stream, param_type );

                    switch( param_type )
                    {
                    case 0:
                        {
                        }break;
                    default:
                        {
                            AE_RETURN_ERROR_RESULT( AE_RESULT_INVALID_STREAM );
                        }
                    }

                    if( param_type == 0 )
                    {
                        break;
                    }
                }

                *it_resource = (aeMovieResource *)resource;

                resource->type = type;
                resource->data = (*_movieData->resource_provider)(*it_resource, _movieData->resource_ud);
            }break;
        case AE_MOVIE_RESOURCE_IMAGE:
            {
                aeMovieResourceImage * resource = AE_NEW( instance, aeMovieResourceImage );

                AE_RESULT_PANIC_MEMORY( resource );

                AE_READ_STRING( _stream, resource->path );
                resource->codec = AE_READ8( _stream );

                resource->premultiplied = AE_READB( _stream );

                AE_READF( _stream, resource->base_width );
                AE_READF( _stream, resource->base_height );

                resource->trim_width = resource->base_width;
                resource->trim_height = resource->base_height;
                resource->offset_x = 0.f;
                resource->offset_y = 0.f;
                resource->uv = instance->sprite_uv;
                resource->mesh = AE_NULL;

                for( ;;)
                {
                    ae_uint8_t param_type;
                    AE_READ( _stream, param_type );

                    switch( param_type )
                    {
                    case 0:
                        {
                        }break;
                    case 1:
                        {
                            AE_READF( _stream, resource->trim_width );
                            AE_READF( _stream, resource->trim_height );
                            AE_READF( _stream, resource->offset_x );
                            AE_READF( _stream, resource->offset_y );
                        }break;
                    case 2:
                        {
                            ae_vector2_t * uv = AE_NEWN( instance, ae_vector2_t, 4 );

                            AE_RESULT_PANIC_MEMORY( uv );

                            AE_READF2( _stream, uv[0] );
                            AE_READF2( _stream, uv[1] );
                            AE_READF2( _stream, uv[2] );
                            AE_READF2( _stream, uv[3] );

                            resource->uv = (const ae_vector2_t *)uv;
                        }break;
                    case 3:
                        {
                            ae_mesh_t * mesh = AE_NEW( instance, ae_mesh_t );

                            AE_RESULT_PANIC_MEMORY( mesh );

                            AE_READ_MESH( _stream, mesh );

                            resource->mesh = mesh;
                        }break;
                    default:
                        {
                            AE_RETURN_ERROR_RESULT( AE_RESULT_INVALID_STREAM );
                        }
                    }

                    if( param_type == 0 )
                    {
                        break;
                    }
                }

                *it_resource = (aeMovieResource *)resource;

                resource->type = type;
                resource->data = (*_movieData->resource_provider)(*it_resource, _movieData->resource_ud);
            }break;
        case AE_MOVIE_RESOURCE_SEQUENCE:
            {
                aeMovieResourceSequence * resource = AE_NEW( instance, aeMovieResourceSequence );

                AE_RESULT_PANIC_MEMORY( resource );

                AE_READF( _stream, resource->frameDurationInv );

                ae_uint32_t image_count = AE_READZ( _stream );

                resource->image_count = image_count;
                const aeMovieResourceImage ** images = AE_NEWN( instance, const aeMovieResourceImage *, image_count );

                AE_RESULT_PANIC_MEMORY( images );

                const aeMovieResourceImage ** it_image = images;
                const aeMovieResourceImage ** it_image_end = images + image_count;
                for( ; it_image != it_image_end; ++it_image )
                {
                    ae_uint32_t resource_id = AE_READZ( _stream );

                    *it_image = (const aeMovieResourceImage *)resources[resource_id];
                }

                resource->images = images;

                for( ;;)
                {
                    ae_uint8_t param_type;
                    AE_READ( _stream, param_type );

                    switch( param_type )
                    {
                    case 0:
                        {
                        }break;
                    default:
                        {
                            AE_RETURN_ERROR_RESULT( AE_RESULT_INVALID_STREAM );
                        }
                    }

                    if( param_type == 0 )
                    {
                        break;
                    }
                }

                *it_resource = (aeMovieResource *)resource;

                resource->type = type;
                resource->data = (*_movieData->resource_provider)(*it_resource, _movieData->resource_ud);
            }break;
        case AE_MOVIE_RESOURCE_PARTICLE:
            {
                aeMovieResourceParticle * resource = AE_NEW( instance, aeMovieResourceParticle );

                AE_RESULT_PANIC_MEMORY( resource );

                AE_READ_STRING( _stream, resource->path );
                resource->codec = AE_READ8( _stream );

                ae_uint32_t image_count = AE_READZ( _stream );

                resource->image_count = image_count;
                const aeMovieResourceImage ** images = AE_NEWN( instance, const aeMovieResourceImage *, image_count );

                AE_RESULT_PANIC_MEMORY( images );

                const aeMovieResourceImage ** it_image = images;
                const aeMovieResourceImage ** it_image_end = images + image_count;
                for( ; it_image != it_image_end; ++it_image )
                {
                    ae_uint32_t resource_id = AE_READZ( _stream );

                    *it_image = (const aeMovieResourceImage *)resources[resource_id];
                }

                resource->images = images;

                for( ;;)
                {
                    ae_uint8_t param_type;
                    AE_READ( _stream, param_type );

                    switch( param_type )
                    {
                    case 0:
                        {
                        }break;
                    default:
                        {
                            AE_RETURN_ERROR_RESULT( AE_RESULT_INVALID_STREAM );
                        }
                    }

                    if( param_type == 0 )
                    {
                        break;
                    }
                }

                *it_resource = (aeMovieResource *)resource;

                resource->type = type;
                resource->data = (*_movieData->resource_provider)(*it_resource, _movieData->resource_ud);
            }break;
        case AE_MOVIE_RESOURCE_SLOT:
            {
                aeMovieResourceSlot * resource = AE_NEW( instance, aeMovieResourceSlot );

                AE_RESULT_PANIC_MEMORY( resource );

                AE_READF( _stream, resource->width );
                AE_READF( _stream, resource->height );

                for( ;;)
                {
                    ae_uint8_t param_type;
                    AE_READ( _stream, param_type );

                    switch( param_type )
                    {
                    case 0:
                        {
                        }break;
                    default:
                        {
                            AE_RETURN_ERROR_RESULT( AE_RESULT_INVALID_STREAM );
                        }
                    }

                    if( param_type == 0 )
                    {
                        break;
                    }
                }

                *it_resource = (aeMovieResource *)resource;

                resource->type = type;
                resource->data = (*_movieData->resource_provider)(*it_resource, _movieData->resource_ud);
            }break;
        default:
            {
                AE_RETURN_ERROR_RESULT( AE_RESULT_INVALID_STREAM );
            }break;
        }
    }

    _movieData->resources = resources;

    ae_uint32_t composition_count = AE_READZ( _stream );

    _movieData->composition_count = composition_count;

    aeMovieCompositionData * compositions = AE_NEWN( instance, aeMovieCompositionData, composition_count );

    AE_RESULT_PANIC_MEMORY( compositions );

    aeMovieCompositionData * it_composition = compositions;
    aeMovieCompositionData * it_composition_end = compositions + composition_count;
    for( ; it_composition != it_composition_end; ++it_composition )
    {
        AE_RESULT( __load_movie_data_composition, (_movieData, compositions, _stream, it_composition) );
    }

    _movieData->compositions = compositions;

    return AE_RESULT_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
const ae_char_t * ae_get_movie_name( const aeMovieData * _movieData )
{
    const ae_char_t * name = _movieData->name;

    return name;
}
//////////////////////////////////////////////////////////////////////////
ae_bool_t ae_has_movie_composition_data( const aeMovieData * _movieData, const ae_char_t * _name )
{
    const aeMovieInstance * instance = _movieData->instance;

    const aeMovieCompositionData * it_composition = _movieData->compositions;
    const aeMovieCompositionData * it_composition_end = _movieData->compositions + _movieData->composition_count;
    for( ; it_composition != it_composition_end; ++it_composition )
    {
        const aeMovieCompositionData * composition = it_composition;

        if( AE_STRNCMP( instance, composition->name, _name, AE_MOVIE_MAX_COMPOSITION_NAME ) != 0 )
        {
            continue;
        }

        return AE_TRUE;
    }

    return AE_FALSE;
}
//////////////////////////////////////////////////////////////////////////
const aeMovieCompositionData * ae_get_movie_composition_data( const aeMovieData * _movieData, const ae_char_t * _name )
{
    const aeMovieInstance * instance = _movieData->instance;

    const aeMovieCompositionData * it_composition = _movieData->compositions;
    const aeMovieCompositionData * it_composition_end = _movieData->compositions + _movieData->composition_count;
    for( ; it_composition != it_composition_end; ++it_composition )
    {
        const aeMovieCompositionData * composition = it_composition;

        if( AE_STRNCMP( instance, composition->name, _name, AE_MOVIE_MAX_COMPOSITION_NAME ) != 0 )
        {
            continue;
        }

        return composition;
    }

    return AE_NULL;
}
//////////////////////////////////////////////////////////////////////////
const ae_char_t * ae_get_movie_layer_data_name( const aeMovieLayerData * _layer )
{
    return _layer->name;
}
//////////////////////////////////////////////////////////////////////////
aeMovieLayerTypeEnum ae_get_movie_layer_data_type( const aeMovieLayerData * _layer )
{
    return _layer->type;
}
//////////////////////////////////////////////////////////////////////////
ae_bool_t ae_is_movie_layer_data_track_mate( const aeMovieLayerData * _layer )
{
    return _layer->is_track_matte;
}
//////////////////////////////////////////////////////////////////////////
ae_bool_t ae_is_movie_layer_data_threeD( const aeMovieLayerData * _layer )
{
    return _layer->threeD;
}
//////////////////////////////////////////////////////////////////////////
ae_bool_t ae_has_movie_layer_data_param( const aeMovieLayerData * _layer, ae_uint32_t _param )
{
    return ((_layer->params & _param) == _param);
}
//////////////////////////////////////////////////////////////////////////
const aeMovieResource * ae_get_movie_layer_data_resource( const aeMovieLayerData * _layer )
{
    return _layer->resource;
}
//////////////////////////////////////////////////////////////////////////
ae_voidptr_t ae_get_movie_layer_data_resource_data( const aeMovieLayerData * _layer )
{
    return _layer->resource->data;
}
//////////////////////////////////////////////////////////////////////////
ae_track_matte_mode_t ae_get_movie_layer_data_track_matte_mode( const aeMovieLayerData * _layer )
{
    return _layer->track_matte_mode;
}
//////////////////////////////////////////////////////////////////////////
ae_blend_mode_t ae_get_movie_layer_data_blend_mode( const aeMovieLayerData * _layer )
{
    return _layer->blend_mode;
}
//////////////////////////////////////////////////////////////////////////
ae_float_t ae_get_movie_layer_data_stretch( const aeMovieLayerData * _layer )
{
    return _layer->stretch;
}
//////////////////////////////////////////////////////////////////////////
ae_bool_t ae_get_movie_layer_data_socket_polygon( const aeMovieLayerData * _layerData, ae_uint32_t _frame, const ae_polygon_t ** _polygon )
{
#ifdef AE_MOVIE_DEBUG
    if( _layerData->type != AE_MOVIE_LAYER_TYPE_SOCKET )
    {
        return AE_FALSE;
    }

    if( _frame >= _layerData->frame_count )
    {
        return AE_FALSE;
    }
#endif

    const aeMovieLayerPolygon * polygon = _layerData->extensions->polygon;

    if( polygon->immutable == AE_TRUE )
    {
        *_polygon = &polygon->immutable_polygon;
    }
    else
    {
        *_polygon = polygon->polygons + _frame;
    }

    return AE_TRUE;
}
//////////////////////////////////////////////////////////////////////////
const ae_char_t * ae_get_movie_composition_data_name( const aeMovieCompositionData * _compositionData )
{
    return _compositionData->name;
}
//////////////////////////////////////////////////////////////////////////
ae_float_t ae_get_movie_composition_data_width( const aeMovieCompositionData * _compositionData )
{
    return _compositionData->width;
}
//////////////////////////////////////////////////////////////////////////
ae_float_t ae_get_movie_composition_data_height( const aeMovieCompositionData * _compositionData )
{
    return _compositionData->height;
}
//////////////////////////////////////////////////////////////////////////
ae_time_t ae_get_movie_composition_data_duration( const aeMovieCompositionData * _compositionData )
{
    return AE_TIME_OUTSCALE( _compositionData->duration );
}
//////////////////////////////////////////////////////////////////////////
ae_time_t ae_get_movie_composition_data_frame_duration( const aeMovieCompositionData * _compositionData )
{
    return AE_TIME_OUTSCALE( _compositionData->frameDuration );
}
//////////////////////////////////////////////////////////////////////////
ae_uint32_t ae_get_movie_composition_data_frame_count( const aeMovieCompositionData * _compositionData )
{
    return _compositionData->frameCount;
}
//////////////////////////////////////////////////////////////////////////
ae_void_t ae_get_movie_composition_data_loop_segment( const aeMovieCompositionData * _compositionData, ae_time_t * _in, ae_time_t * _out )
{
    *_in = AE_TIME_OUTSCALE( _compositionData->loop_segment[0] );
    *_out = AE_TIME_OUTSCALE( _compositionData->loop_segment[1] );
}
//////////////////////////////////////////////////////////////////////////
ae_bool_t ae_get_movie_composition_data_master( const aeMovieCompositionData * _compositionData )
{
    return _compositionData->master;
}
//////////////////////////////////////////////////////////////////////////
ae_uint32_t ae_get_movie_composition_data_count( const aeMovieData * _movieData )
{
    return _movieData->composition_count;
}
//////////////////////////////////////////////////////////////////////////
const aeMovieCompositionData * ae_get_movie_composition_data_by_index( const aeMovieData * _movieData, ae_uint32_t _index )
{
    return _movieData->compositions + _index;
}
//////////////////////////////////////////////////////////////////////////
ae_uint32_t ae_get_movie_composition_data_event_count( const aeMovieCompositionData * _compositionData )
{
    ae_uint32_t count = 0;

    const aeMovieLayerData * it_layer = _compositionData->layers;
    const aeMovieLayerData * it_layer_end = _compositionData->layers + _compositionData->layer_count;
    for( ; it_layer != it_layer_end; ++it_layer )
    {
        aeMovieLayerTypeEnum type = it_layer->type;

        if( type == AE_MOVIE_LAYER_TYPE_EVENT )
        {
            ++count;
        }
        else if( type == AE_MOVIE_LAYER_TYPE_MOVIE )
        {
            count += ae_get_movie_composition_data_event_count( it_layer->sub_composition_data );
        }
    }

    return count;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_bool_t __ae_get_composition_data_event_name( const aeMovieCompositionData * _compositionData, ae_uint32_t * _iterator, ae_uint32_t _index, const ae_char_t ** _name )
{
    const aeMovieLayerData * it_layer = _compositionData->layers;
    const aeMovieLayerData * it_layer_end = _compositionData->layers + _compositionData->layer_count;
    for( ; it_layer != it_layer_end; ++it_layer )
    {
        aeMovieLayerTypeEnum type = it_layer->type;

        if( type == AE_MOVIE_LAYER_TYPE_EVENT )
        {
            if( (*_iterator)++ == _index )
            {
                *_name = it_layer->name;

                return AE_TRUE;
            }
        }
        else if( type == AE_MOVIE_LAYER_TYPE_MOVIE )
        {
            if( __ae_get_composition_data_event_name( it_layer->sub_composition_data, _iterator, _index, _name ) == AE_TRUE )
            {
                return AE_TRUE;
            }
        }
    }

    return AE_FALSE;
}
//////////////////////////////////////////////////////////////////////////
const ae_char_t * ae_get_movie_composition_data_event_name( const aeMovieCompositionData * _compositionData, ae_uint32_t _index )
{
    ae_uint32_t iterator = 0U;
    const ae_char_t * name;
    if( __ae_get_composition_data_event_name( _compositionData, &iterator, _index, &name ) == AE_FALSE )
    {
        return AE_NULL;
    }

    return name;
}
//////////////////////////////////////////////////////////////////////////