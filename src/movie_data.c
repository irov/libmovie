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

#	include "movie/movie_data.h"
#	include "movie/movie_resource.h"

#	include "movie_transformation.h"
#	include "movie_memory.h"
#	include "movie_stream.h"

//////////////////////////////////////////////////////////////////////////
static const ae_uint32_t ae_movie_version = 12;
//////////////////////////////////////////////////////////////////////////
aeMovieData * ae_create_movie_data( const aeMovieInstance * _instance, ae_movie_data_resource_provider_t _provider, ae_movie_data_resource_deleter_t _deleter, ae_voidptr_t _data )
{
    aeMovieData * m = AE_NEW( _instance, aeMovieData );

    m->instance = _instance;

    m->name = "";

    m->resource_provider = _provider;
    m->resource_deleter = _deleter;
    m->resource_ud = _data;

    m->resource_count = 0;
    m->resources = AE_NULL;

    m->composition_count = 0;
    m->compositions = AE_NULL;

    return m;
}
//////////////////////////////////////////////////////////////////////////
static void __ae_delete_mesh_t( const aeMovieInstance * _instance, const aeMovieMesh * _mesh )
{
    AE_DELETEN( _instance, _mesh->positions );
    AE_DELETEN( _instance, _mesh->uvs );
    AE_DELETEN( _instance, _mesh->indices );
}
//////////////////////////////////////////////////////////////////////////
static void __ae_delete_layer_mesh_t( const aeMovieInstance * _instance, const aeMovieLayerMesh * _layerMesh, ae_uint32_t _count )
{
    if( _layerMesh->immutable == AE_TRUE )
    {
        __ae_delete_mesh_t( _instance, &_layerMesh->immutable_mesh );
    }
    else
    {
        const aeMovieMesh * it_mesh = _layerMesh->meshes;
        const aeMovieMesh * it_mesh_end = _layerMesh->meshes + _count;
        for( ; it_mesh != it_mesh_end; ++it_mesh )
        {
            const aeMovieMesh * mesh = it_mesh;

            __ae_delete_mesh_t( _instance, mesh );
        }

        AE_DELETEN( _instance, _layerMesh->meshes );
    }
}
//////////////////////////////////////////////////////////////////////////
static void __ae_delete_property_value( const aeMovieInstance * _instance, const struct aeMoviePropertyValue * _property )
{
    AE_DELETEN( _instance, _property->values );
}
//////////////////////////////////////////////////////////////////////////
static void __ae_delete_property_color( const aeMovieInstance * _instance, const struct aeMoviePropertyColor * _property )
{
    AE_DELETEN( _instance, _property->colors_r );
    AE_DELETEN( _instance, _property->colors_g );
    AE_DELETEN( _instance, _property->colors_b );
}
//////////////////////////////////////////////////////////////////////////
void ae_delete_movie_data( const aeMovieData * _movieData )
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

                (void)resource;

                (*_movieData->resource_deleter)( type, base_resource->data, _movieData->resource_ud);

            }break;
        case AE_MOVIE_RESOURCE_VIDEO:
            {
                const aeMovieResourceVideo * resource = (const aeMovieResourceVideo *)base_resource;

                AE_DELETEN( instance, resource->path );

                (*_movieData->resource_deleter)(type, base_resource->data, _movieData->resource_ud);

            }break;
        case AE_MOVIE_RESOURCE_SOUND:
            {
                const aeMovieResourceSound * resource = (const aeMovieResourceSound *)base_resource;

                AE_DELETEN( instance, resource->path );

                (*_movieData->resource_deleter)(type, base_resource->data, _movieData->resource_ud);

            }break;
        case AE_MOVIE_RESOURCE_IMAGE:
            {
                const aeMovieResourceImage * resource = (const aeMovieResourceImage *)base_resource;

                AE_DELETEN( instance, resource->path );

                if( resource->uv != _movieData->instance->sprite_uv )
                {
                    AE_DELETE( instance, resource->uv );
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

                AE_DELETEN( instance, resource->path );

                (*_movieData->resource_deleter)(type, base_resource->data, _movieData->resource_ud);

            }break;
        case AE_MOVIE_RESOURCE_SLOT:
            {
                const aeMovieResourceSlot * resource = (const aeMovieResourceSlot *)base_resource;

                (void)resource;

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

        const aeMovieCompositionCamera * camera = composition->camera;

        if( camera != AE_NULL )
        {
            AE_DELETE( instance, camera->name );
            AE_DELETE( instance, camera );
        }

        const aeMovieLayerData * it_layer = composition->layers;
        const aeMovieLayerData * it_layer_end = composition->layers + composition->layer_count;
        for( ; it_layer != it_layer_end; ++it_layer )
        {
            const aeMovieLayerData * layer = it_layer;

            if( layer->timeremap != AE_NULL )
            {
                AE_DELETEN( instance, layer->timeremap->times );

                AE_DELETEN( instance, layer->timeremap );
            }

            if( layer->mesh != AE_NULL )
            {
                __ae_delete_layer_mesh_t( instance, layer->mesh, layer->frame_count );

                AE_DELETEN( instance, layer->mesh );
            }

            if( layer->bezier_warp != AE_NULL )
            {
                AE_DELETEN( instance, layer->bezier_warp->bezier_warps );

                AE_DELETEN( instance, layer->bezier_warp );
            }

            if( layer->color_vertex != AE_NULL )
            {
                const struct aeMoviePropertyColor * property_color = layer->color_vertex->property_color;

                __ae_delete_property_color( instance, property_color );

                AE_DELETEN( instance, property_color );

                AE_DELETEN( instance, layer->color_vertex );
            }

            if( layer->polygon != AE_NULL )
            {
                AE_DELETEN( instance, layer->polygon->polygons );

                AE_DELETEN( instance, layer->polygon );
            }

            if( layer->shader != AE_NULL )
            {
                const aeMovieLayerShader * shader = layer->shader;

                AE_DELETE( instance, shader->name );
                AE_DELETE( instance, shader->shader_vertex );
                AE_DELETE( instance, shader->shader_fragment );

                const struct aeMovieLayerShaderParameter ** it_parameter = shader->parameters;
                const struct aeMovieLayerShaderParameter ** it_parameter_end = shader->parameters + shader->parameter_count;

                for( ;
                    it_parameter != it_parameter_end;
                    ++it_parameter )
                {
                    const struct aeMovieLayerShaderParameter * parameter = *it_parameter;

                    AE_DELETE( instance, parameter->name );

                    switch( parameter->type )
                    {
                    case 3:
                        {
                            const struct aeMovieLayerShaderParameterSlider * parameter_slider = (const struct aeMovieLayerShaderParameterSlider *)parameter;

                            __ae_delete_property_value( instance, parameter_slider->property_value );

                            AE_DELETE( instance, parameter_slider->property_value );
                        }break;
                    case 5:
                        {
                            const struct aeMovieLayerShaderParameterColor * parameter_color = (const struct aeMovieLayerShaderParameterColor *)parameter;

                            __ae_delete_property_color( instance, parameter_color->property_color );

                            AE_DELETE( instance, parameter_color->property_color );
                        }break;
                    }
                }

                AE_DELETEN( instance, shader );
            }

            ae_movie_delete_layer_transformation( instance, layer->transformation, layer->threeD );

            AE_DELETE( instance, layer->transformation );

            AE_DELETEN( instance, layer->name );
        }

        AE_DELETEN( instance, composition->layers );

        AE_DELETEN( instance, composition->name );
    }

    AE_DELETEN( instance, _movieData->resources );
    AE_DELETEN( instance, _movieData->compositions );

    AE_DELETEN( instance, _movieData->name );

    AE_DELETE( instance, _movieData );
}
//////////////////////////////////////////////////////////////////////////
static void __load_movie_data_composition_camera( aeMovieStream * _stream, aeMovieCompositionData * _compositionData )
{
    aeMovieCompositionCamera * camera = AE_NEW( _stream->instance, aeMovieCompositionCamera );

    ae_bool_t export_camera = READB( _stream );

    if( export_camera == AE_FALSE )
    {
        camera->name = AE_NEWN( _stream->instance, ae_char_t, 5 );
        camera->name[0] = 'N';
        camera->name[1] = 'o';
        camera->name[2] = 'n';
        camera->name[3] = 'e';
        camera->name[4] = 0;
        
        READ( _stream, camera->zoom );
        READ( _stream, camera->fov );

        ae_float_t width = _compositionData->width;
        ae_float_t height = _compositionData->height;

        camera->immutable_property_mask = __AE_MOVIE_IMMUTABLE_CAMERA_ALL__;

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
    }
    else
    {
        READ_STRING( _stream, camera->name );

        READ( _stream, camera->zoom );
        READ( _stream, camera->fov );

        ae_movie_load_camera_transformation( _stream, camera );
    }

    _compositionData->camera = camera;
}
//////////////////////////////////////////////////////////////////////////
static ae_uint32_t __find_movie_data_composition_layer_position_by_index( const aeMovieCompositionData * _compositionData, const aeMovieLayerData * _layers, ae_uint32_t _index )
{
    ae_uint32_t iterator = 0;

    const aeMovieLayerData * it_layer = _layers;
    const aeMovieLayerData * it_layer_end = _layers + _compositionData->layer_count;
    for( ; it_layer != it_layer_end; ++it_layer )
    {
        const aeMovieLayerData * layer = it_layer;

        if( layer->index == _index )
        {
            return iterator;
        }

        ++iterator;
    }

    return (ae_uint32_t)-1;
}
//////////////////////////////////////////////////////////////////////////
static ae_result_t __setup_movie_data_layer_track_matte( const aeMovieCompositionData * _compositionData, const aeMovieLayerData * _layers, aeMovieLayerData * _layer )
{
    if( _layer->has_track_matte == AE_TRUE )
    {
        ae_uint32_t layer_position = __find_movie_data_composition_layer_position_by_index( _compositionData, _layers, _layer->index );

        if( layer_position == (ae_uint32_t)-1 )
        {
            return AE_MOVIE_FAILED;
        }

        if( layer_position + 1 >= _compositionData->layer_count )
        {
            return AE_MOVIE_FAILED;
        }

        _layer->track_matte_layer = _compositionData->layers + layer_position + 1;
    }
    else
    {
        _layer->track_matte_layer = AE_NULL;
    }

    return AE_MOVIE_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static void __load_movie_property_color( aeMovieStream * _stream, struct aeMoviePropertyColor * _property, const aeMovieLayerData * _layer )
{
    _property->immutable_r = READB( _stream );

    if( _property->immutable_r == AE_TRUE )
    {
        READ( _stream, _property->immutable_color_r );

        _property->colors_r = AE_NULL;
    }
    else
    {
        _property->immutable_color_r = 1.f;

        ae_color_t * colors_r = AE_NEWN( _stream->instance, ae_color_t, _layer->frame_count );
        READN( _stream, colors_r, _layer->frame_count );

        _property->colors_r = colors_r;
    }

    _property->immutable_g = READB( _stream );

    if( _property->immutable_g == AE_TRUE )
    {
        READ( _stream, _property->immutable_color_g );

        _property->colors_g = AE_NULL;
    }
    else
    {
        _property->immutable_color_g = 1.f;

        ae_color_t * colors_g = AE_NEWN( _stream->instance, ae_color_t, _layer->frame_count );
        READN( _stream, colors_g, _layer->frame_count );

        _property->colors_g = colors_g;
    }

    _property->immutable_b = READB( _stream );

    if( _property->immutable_b == AE_TRUE )
    {
        READ( _stream, _property->immutable_color_b );

        _property->colors_b = AE_NULL;
    }
    else
    {
        _property->immutable_color_b = 1.f;

        ae_color_t * colors_b = AE_NEWN( _stream->instance, ae_color_t, _layer->frame_count );
        READN( _stream, colors_b, _layer->frame_count );

        _property->colors_b = colors_b;
    }
}
//////////////////////////////////////////////////////////////////////////
static void __load_movie_property_value( aeMovieStream * _stream, struct aeMoviePropertyValue * _property, const aeMovieLayerData * _layer )
{
    _property->immutable = READB( _stream );

    if( _property->immutable == AE_TRUE )
    {
        READ( _stream, _property->immutable_value );

        _property->values = AE_NULL;
    }
    else
    {
        _property->immutable_value = 0.f;

        ae_float_t * values = AE_NEWN( _stream->instance, ae_float_t, _layer->frame_count );
        READN( _stream, values, _layer->frame_count );

        _property->values = values;
    }
}
//////////////////////////////////////////////////////////////////////////
static ae_result_t __load_movie_data_layer( const aeMovieData * _movieData, const aeMovieCompositionData * _compositions, aeMovieStream * _stream, const aeMovieCompositionData * _compositionData, aeMovieLayerData * _layer )
{
    const aeMovieInstance * instance = _movieData->instance;

    READ_STRING( _stream, _layer->name );

    _layer->index = READZ( _stream );

    _layer->is_track_matte = READB( _stream );
    _layer->has_track_matte = READB( _stream );

    ae_uint8_t type;
    READ( _stream, type );
    _layer->type = type;

    _layer->frame_count = READZ( _stream );

    _layer->timeremap = AE_NULL;
    _layer->mesh = AE_NULL;
    _layer->bezier_warp = AE_NULL;
    _layer->color_vertex = AE_NULL;
    _layer->polygon = AE_NULL;
    _layer->shader = AE_NULL;

    for( ;; )
    {
        ae_uint8_t extension;
        READ( _stream, extension );

        switch( extension )
        {
        case 0:
            {
            }break;
        case 1:
            {
                aeMovieLayerTimeremap * layer_timeremap = AE_NEW( instance, aeMovieLayerTimeremap );

                ae_float_t * times = AE_NEWN( instance, ae_float_t, _layer->frame_count );
                READN( _stream, times, _layer->frame_count );

                layer_timeremap->times = times;

                _layer->timeremap = layer_timeremap;
            }break;
        case 2:
            {
                aeMovieLayerMesh * layer_mesh = AE_NEW( instance, aeMovieLayerMesh );

                layer_mesh->immutable = READB( _stream );

                if( layer_mesh->immutable == AE_TRUE )
                {
                    READ_MESH( _stream, &layer_mesh->immutable_mesh );

                    layer_mesh->meshes = AE_NULL;
                }
                else
                {
                    aeMovieMesh * meshes = AE_NEWN( instance, aeMovieMesh, _layer->frame_count );

                    aeMovieMesh * it_mesh = meshes;
                    aeMovieMesh * it_mesh_end = meshes + _layer->frame_count;
                    for( ; it_mesh != it_mesh_end; ++it_mesh )
                    {
                        READ_MESH( _stream, it_mesh );
                    }

                    layer_mesh->meshes = meshes;
                }

                _layer->mesh = layer_mesh;
            }break;
        case 3:
            {
                aeMovieLayerBezierWarp * layer_bezier_warp = AE_NEW( instance, aeMovieLayerBezierWarp );

                layer_bezier_warp->immutable = READB( _stream );

                if( layer_bezier_warp->immutable == AE_TRUE )
                {
                    READN( _stream, layer_bezier_warp->immutable_bezier_warp.corners, 4 );
                    READN( _stream, layer_bezier_warp->immutable_bezier_warp.beziers, 8 );

                    layer_bezier_warp->bezier_warps = AE_NULL;
                }
                else
                {
                    aeMovieBezierWarp * bezier_warps = AE_NEWN( instance, aeMovieBezierWarp, _layer->frame_count );

                    aeMovieBezierWarp * it_bezier_warp = bezier_warps;
                    aeMovieBezierWarp * it_bezier_warp_end = bezier_warps + _layer->frame_count;
                    for( ; it_bezier_warp != it_bezier_warp_end; ++it_bezier_warp )
                    {
                        READN( _stream, it_bezier_warp->corners, 4 );
                        READN( _stream, it_bezier_warp->beziers, 8 );
                    }

                    layer_bezier_warp->bezier_warps = bezier_warps;
                }

                _layer->bezier_warp = layer_bezier_warp;
            }break;
        case 4:
            {
                aeMovieLayerColorVertex * layer_color_vertex = AE_NEW( instance, aeMovieLayerColorVertex );

                struct aeMoviePropertyColor * property_color = AE_NEW( instance, struct aeMoviePropertyColor );

                __load_movie_property_color( _stream, property_color, _layer );

                layer_color_vertex->property_color = property_color;

                _layer->color_vertex = layer_color_vertex;
            }break;
        case 5:
            {
                aeMovieLayerPolygon * layer_polygon = AE_NEW( instance, aeMovieLayerPolygon );
                layer_polygon->immutable = READB( _stream );

                if( layer_polygon->immutable == AE_TRUE )
                {
                    READ_POLYGON( _stream, &layer_polygon->immutable_polygon );

                    layer_polygon->polygons = AE_NULL;
                }
                else
                {
                    ae_uint32_t polygon_count = READZ( _stream );

                    aeMoviePolygon * polygons = AE_NEWN( instance, aeMoviePolygon, polygon_count );

                    aeMoviePolygon * it_polygon = polygons;
                    aeMoviePolygon * it_polygon_end = polygons + polygon_count;
                    for( ; it_polygon != it_polygon_end; ++it_polygon )
                    {
                        READ_POLYGON( _stream, it_polygon );
                    }

                    layer_polygon->polygons = polygons;
                }

                _layer->polygon = layer_polygon;
            }
        case 6:
            {
                aeMovieLayerShader * layer_shader = AE_NEW( instance, aeMovieLayerShader );

                READ_STRING( _stream, layer_shader->name );
                READ( _stream, layer_shader->version );

                READ_STRING( _stream, layer_shader->shader_vertex );
                READ_STRING( _stream, layer_shader->shader_fragment );

                READ( _stream, layer_shader->parameter_count );

                const struct aeMovieLayerShaderParameter ** parameters = AE_NEWN( _stream->instance, const struct aeMovieLayerShaderParameter *, layer_shader->parameter_count );

                const struct aeMovieLayerShaderParameter ** it_parameter = parameters;
                const struct aeMovieLayerShaderParameter ** it_parameter_end = parameters + layer_shader->parameter_count;

                for( ;
                    it_parameter != it_parameter_end;
                    ++it_parameter )
                {
                    uint32_t paramater_type;
                    READ( _stream, paramater_type );

                    switch( paramater_type )
                    {
                    case 3:
                        {
                            struct aeMovieLayerShaderParameterSlider * parameter_slider = AE_NEW( _stream->instance, struct aeMovieLayerShaderParameterSlider );

                            parameter_slider->type = paramater_type;
                            READ_STRING( _stream, parameter_slider->name );

                            struct aeMoviePropertyValue * property_value = AE_NEW( _stream->instance, struct aeMoviePropertyValue );

                            __load_movie_property_value( _stream, property_value, _layer );

                            parameter_slider->property_value = property_value;

                            *it_parameter = (struct aeMovieLayerShaderParameter *)parameter_slider;
                        }break;
                    case 5:
                        {
                            struct aeMovieLayerShaderParameterColor * parameter_color = AE_NEW( _stream->instance, struct aeMovieLayerShaderParameterColor );

                            parameter_color->type = paramater_type;
                            READ_STRING( _stream, parameter_color->name );

                            struct aeMoviePropertyColor * property_color = AE_NEW( _stream->instance, struct aeMoviePropertyColor );

                            __load_movie_property_color( _stream, property_color, _layer );

                            parameter_color->property_color = property_color;

                            *it_parameter = (struct aeMovieLayerShaderParameter *)parameter_color;
                        }break;
                    }
                }

                layer_shader->parameters = parameters;

                _layer->shader = layer_shader;
            }break;
        default:
            {
                return AE_MOVIE_FAILED;
            }break;
        }

        if( extension == 0 )
        {
            break;
        }
    }

    ae_bool_t is_resource_or_composition = READB( _stream );

    if( is_resource_or_composition == AE_TRUE )
    {
        ae_uint32_t resource_index = READZ( _stream );

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
        ae_uint32_t composition_index = READZ( _stream );
        _layer->sub_composition_data = _compositions + composition_index;

        _layer->resource = AE_NULL;
    }

    ae_uint32_t parent_index = READZ( _stream );

    _layer->parent_index = parent_index;

    _layer->reverse_time = READB( _stream );

    READ( _stream, _layer->start_time );
    READ( _stream, _layer->in_time );
    READ( _stream, _layer->out_time );

    if( _layer->in_time < 0.f )
    {
        _layer->start_time -= _layer->in_time;
        _layer->in_time = 0.f;
    }

    if( _layer->out_time > _compositionData->duration )
    {
        _layer->out_time = _compositionData->duration;
    }

    ae_uint8_t blend_mode;
    READ( _stream, blend_mode );
    _layer->blend_mode = blend_mode;

    _layer->threeD = READB( _stream );
    READ( _stream, _layer->params );

    _layer->play_count = READZ( _stream );

    READ( _stream, _layer->stretch );

    aeMovieLayerTransformation * transformation = AE_NULL;

    if( _layer->threeD == AE_FALSE )
    {
        transformation = (aeMovieLayerTransformation *)AE_NEW( instance, aeMovieLayerTransformation2D );
    }
    else
    {
        transformation = (aeMovieLayerTransformation *)AE_NEW( instance, aeMovieLayerTransformation3D );
    }

    if( ae_movie_load_layer_transformation( _stream, transformation, _layer->threeD ) == AE_MOVIE_FAILED )
    {
        return AE_MOVIE_FAILED;
    }

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
                return AE_MOVIE_FAILED;
            }break;
        }
    }

    return AE_MOVIE_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static ae_result_t __load_movie_data_composition_layers( const aeMovieData * _movieData, const aeMovieCompositionData * _compositions, aeMovieLayerData * _layers, aeMovieStream * _stream, aeMovieCompositionData * _compositionData )
{
    aeMovieLayerData * it_layer = _layers;
    aeMovieLayerData * it_layer_end = _layers + _compositionData->layer_count;
    for( ; it_layer != it_layer_end; ++it_layer )
    {
        aeMovieLayerData * layer = it_layer;

        layer->composition_data = _compositionData;

        if( __load_movie_data_layer( _movieData, _compositions, _stream, _compositionData, layer ) == AE_MOVIE_FAILED )
        {
            return AE_MOVIE_FAILED;
        }
    }

    return AE_MOVIE_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static ae_result_t __setup_movie_data_composition_layers( const aeMovieCompositionData * _compositionData, aeMovieLayerData * _layers )
{
    aeMovieLayerData * it_layer = _layers;
    aeMovieLayerData * it_layer_end = _layers + _compositionData->layer_count;
    for( ; it_layer != it_layer_end; ++it_layer )
    {
        aeMovieLayerData * layer = it_layer;

        if( __setup_movie_data_layer_track_matte( _compositionData, _layers, layer ) == AE_MOVIE_FAILED )
        {
            return AE_MOVIE_FAILED;
        }
    }

    return AE_MOVIE_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static ae_result_t __load_movie_data_composition( const aeMovieData * _movieData, const aeMovieCompositionData * _compositions, aeMovieStream * _stream, aeMovieCompositionData * _compositionData )
{
    READ_STRING( _stream, _compositionData->name );

    _compositionData->master = READB( _stream );

    READ( _stream, _compositionData->width );
    READ( _stream, _compositionData->height );

    READ( _stream, _compositionData->duration );
    READ( _stream, _compositionData->frameDuration );
    READ( _stream, _compositionData->frameDurationInv );

    _compositionData->camera = AE_NULL;

    _compositionData->frameCount = (ae_uint32_t)(_compositionData->duration * _compositionData->frameDurationInv + 0.5f);

    _compositionData->flags = 0;

    _compositionData->loop_segment[0] = 0.f;
    _compositionData->loop_segment[1] = _compositionData->duration;

    _compositionData->camera = AE_NULL;

    for( ;; )
    {
        ae_uint8_t flag;
        READ( _stream, flag );

        switch( flag )
        {
        case 0:
            {
            }break;
        case 1:
            {
                READ( _stream, _compositionData->loop_segment );

                _compositionData->flags |= AE_MOVIE_COMPOSITION_LOOP_SEGMENT;
            }break;
        case 2:
            {
                READ( _stream, _compositionData->anchor_point );

                _compositionData->flags |= AE_MOVIE_COMPOSITION_ANCHOR_POINT;
            }break;
        case 3:
            {
                READ( _stream, _compositionData->offset_point );

                _compositionData->flags |= AE_MOVIE_COMPOSITION_OFFSET_POINT;
            }break;
        case 4:
            {
                READ( _stream, _compositionData->bounds );

                _compositionData->flags |= AE_MOVIE_COMPOSITION_BOUNDS;
            }break;
        case 5:
            {
                __load_movie_data_composition_camera( _stream, _compositionData );

                _compositionData->flags |= AE_MOVIE_COMPOSITION_CAMERA;
            }break;
        default:
            {
                return AE_MOVIE_FAILED;
            }break;
        };

        if( flag == 0 )
        {
            break;
        }
    }

    ae_uint32_t layer_count = READZ( _stream );

    _compositionData->layer_count = layer_count;
    aeMovieLayerData * layers = AE_NEWN( _movieData->instance, aeMovieLayerData, layer_count );

    if( __load_movie_data_composition_layers( _movieData, _compositions, layers, _stream, _compositionData ) == AE_MOVIE_FAILED )
    {
        return AE_MOVIE_FAILED;
    }

    if( __setup_movie_data_composition_layers( _compositionData, layers ) == AE_MOVIE_FAILED )
    {
        return AE_MOVIE_FAILED;
    }

    _compositionData->layers = layers;

    return AE_MOVIE_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
aeMovieStream * ae_create_movie_stream( const aeMovieInstance * _instance, ae_movie_stream_memory_read_t _read, ae_movie_stream_memory_copy_t _copy, ae_voidptr_t _data )
{
#	ifdef AE_MOVIE_DEBUG
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
#	endif

    aeMovieStream * stream = AE_NEW( _instance, aeMovieStream );

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
static ae_size_t __ae_read_buffer( ae_voidptr_t _data, ae_voidptr_t _buff, ae_uint32_t _carriage, ae_uint32_t _size )
{
    aeMovieStream * stream = (aeMovieStream *)_data;

    stream->memory_copy( stream->copy_data, (ae_constbyteptr_t)stream->buffer + _carriage, _buff, _size );
        
    return _size;
}
//////////////////////////////////////////////////////////////////////////
aeMovieStream * ae_create_movie_stream_memory( const aeMovieInstance * _instance, ae_constvoidptr_t _buffer, ae_movie_stream_memory_copy_t _copy, ae_voidptr_t _data )
{
#	ifdef AE_MOVIE_DEBUG
    if( _instance == AE_NULL )
    {
        return AE_NULL;
    }

    if( _copy == AE_NULL )
    {
        return AE_NULL;
    }
#	endif

    aeMovieStream * stream = AE_NEW( _instance, aeMovieStream );

    stream->instance = _instance;
    stream->memory_read = __ae_read_buffer;
    stream->memory_copy = _copy;
    stream->read_data = stream;
    stream->copy_data = _data;

    stream->buffer = _buffer;
    stream->carriage = 0U;

    return stream;
}
//////////////////////////////////////////////////////////////////////////
void ae_delete_movie_stream( aeMovieStream * _stream )
{
    AE_DELETE( _stream->instance, _stream );
}
//////////////////////////////////////////////////////////////////////////
ae_result_t ae_load_movie_data( aeMovieData * _movieData, aeMovieStream * _stream )
{
    ae_uint8_t magic[4];
    READN( _stream, magic, 4 );

    if( magic[0] != 'A' ||
        magic[1] != 'E' ||
        magic[2] != 'M' ||
        magic[3] != '1' )
    {
        return AE_MOVIE_INVALID_MAGIC;
    }

    ae_uint32_t version;
    READ( _stream, version );

    if( version != ae_movie_version )
    {
        return AE_MOVIE_INVALID_VERSION;
    }

    ae_uint32_t hash_crc;
    READ( _stream, hash_crc );

    ae_uint32_t ae_movie_hashmask_crc =
        _stream->instance->hashmask[0] ^
        _stream->instance->hashmask[1] ^
        _stream->instance->hashmask[2] ^
        _stream->instance->hashmask[3] ^
        _stream->instance->hashmask[4];

    if( hash_crc != ae_movie_hashmask_crc )
    {
        return AE_MOVIE_INVALID_HASH;
    }

    READ_STRING( _stream, _movieData->name );

    ae_uint32_t resource_count = READZ( _stream );

    _movieData->resource_count = resource_count;
    const aeMovieResource ** resources = AE_NEWN( _movieData->instance, const aeMovieResource *, resource_count );

    const aeMovieResource ** it_resource = resources;
    const aeMovieResource ** it_resource_end = resources + resource_count;
    for( ; it_resource != it_resource_end; ++it_resource )
    {
        ae_uint8_t type;
        READ( _stream, type );

        switch( type )
        {
        case AE_MOVIE_RESOURCE_SOLID:
            {
                aeMovieResourceSolid * resource = AE_NEW( _movieData->instance, aeMovieResourceSolid );

                READ( _stream, resource->width );
                READ( _stream, resource->height );
                READ( _stream, resource->r );
                READ( _stream, resource->g );
                READ( _stream, resource->b );

                for( ;;)
                {
                    ae_uint8_t param_type;
                    READ( _stream, param_type );

                    switch( param_type )
                    {
                    case 0:
                        {
                        }break;
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
                aeMovieResourceVideo * resource = AE_NEW( _movieData->instance, aeMovieResourceVideo );

                READ_STRING( _stream, resource->path );
                READ( _stream, resource->codec );

                READ( _stream, resource->width );
                READ( _stream, resource->height );

                ae_uint8_t alpha;
                READ( _stream, alpha );
                resource->alpha = alpha;

                READ( _stream, resource->frameRate );
                READ( _stream, resource->duration );

                for( ;;)
                {
                    ae_uint8_t param_type;
                    READ( _stream, param_type );

                    switch( param_type )
                    {
                    case 0:
                        {
                        }break;
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
                aeMovieResourceSound * resource = AE_NEW( _movieData->instance, aeMovieResourceSound );

                READ_STRING( _stream, resource->path );
                READ( _stream, resource->codec );

                READ( _stream, resource->duration );

                for( ;;)
                {
                    ae_uint8_t param_type;
                    READ( _stream, param_type );

                    switch( param_type )
                    {
                    case 0:
                        {
                        }break;
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
                aeMovieResourceImage * resource = AE_NEW( _movieData->instance, aeMovieResourceImage );

                READ_STRING( _stream, resource->path );
                READ( _stream, resource->codec );

                resource->premultiplied = READB( _stream );

                READ( _stream, resource->base_width );
                READ( _stream, resource->base_height );

                resource->trim_width = resource->base_width;
                resource->trim_height = resource->base_height;
                resource->offset_x = 0.f;
                resource->offset_y = 0.f;
                resource->uv = _movieData->instance->sprite_uv;
                resource->mesh = AE_NULL;

                for( ;;)
                {
                    ae_uint8_t param_type;
                    READ( _stream, param_type );

                    switch( param_type )
                    {
                    case 0:
                        {
                        }break;
                    case 1:
                        {
                            READ( _stream, resource->trim_width );
                            READ( _stream, resource->trim_height );
                            READ( _stream, resource->offset_x );
                            READ( _stream, resource->offset_y );
                        }break;
                    case 2:
                        {
                            ae_vector2_t * uv = AE_NEWN( _movieData->instance, ae_vector2_t, 4 );
                            READ( _stream, uv[0][0] );
                            READ( _stream, uv[0][1] );
                            READ( _stream, uv[1][0] );
                            READ( _stream, uv[1][1] );
                            READ( _stream, uv[2][0] );
                            READ( _stream, uv[2][1] );
                            READ( _stream, uv[3][0] );
                            READ( _stream, uv[3][1] );

                            resource->uv = uv;
                        }break;
                    case 3:
                        {
                            aeMovieMesh * mesh = AE_NEW( _movieData->instance, aeMovieMesh );
                            READ_MESH( _stream, mesh );

                            resource->mesh = mesh;
                        }break;
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
                aeMovieResourceSequence * resource = AE_NEW( _movieData->instance, aeMovieResourceSequence );

                READ( _stream, resource->frameDurationInv );

                ae_uint32_t image_count = READZ( _stream );

                resource->image_count = image_count;
                const aeMovieResourceImage ** images = AE_NEWN( _movieData->instance, const aeMovieResourceImage *, image_count );

                const aeMovieResourceImage ** it_image = images;
                const aeMovieResourceImage ** it_image_end = images + image_count;
                for( ; it_image != it_image_end; ++it_image )
                {
                    ae_uint32_t resource_id = READZ( _stream );

                    *it_image = (const aeMovieResourceImage *)resources[resource_id];
                }

                resource->images = images;

                for( ;;)
                {
                    ae_uint8_t param_type;
                    READ( _stream, param_type );

                    switch( param_type )
                    {
                    case 0:
                        {
                        }break;
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
                aeMovieResourceParticle * resource = AE_NEW( _movieData->instance, aeMovieResourceParticle );

                READ_STRING( _stream, resource->path );
                READ( _stream, resource->codec );

                for( ;;)
                {
                    ae_uint8_t param_type;
                    READ( _stream, param_type );

                    switch( param_type )
                    {
                    case 0:
                        {
                        }break;
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
                aeMovieResourceSlot * resource = AE_NEW( _movieData->instance, aeMovieResourceSlot );

                READ( _stream, resource->width );
                READ( _stream, resource->height );

                for( ;;)
                {
                    ae_uint8_t param_type;
                    READ( _stream, param_type );

                    switch( param_type )
                    {
                    case 0:
                        {
                        }break;
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
                return AE_MOVIE_FAILED;
            }break;
        }
    }

    _movieData->resources = resources;

    ae_uint32_t composition_count = READZ( _stream );

    _movieData->composition_count = composition_count;

    aeMovieCompositionData * compositions = AE_NEWN( _movieData->instance, aeMovieCompositionData, composition_count );

    aeMovieCompositionData * it_composition = compositions;
    aeMovieCompositionData * it_composition_end = compositions + composition_count;
    for( ; it_composition != it_composition_end; ++it_composition )
    {
        if( __load_movie_data_composition( _movieData, compositions, _stream, it_composition ) == AE_MOVIE_FAILED )
        {
            return AE_MOVIE_FAILED;
        }
    }

    _movieData->compositions = compositions;

    return AE_MOVIE_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
const ae_char_t * ae_get_movie_name( const aeMovieData * _movieData )
{
    const ae_char_t * name = _movieData->name;

    return name;
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
aeMovieBlendMode ae_get_movie_layer_data_blend_mode( const aeMovieLayerData * _layer )
{
    return _layer->blend_mode;
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
ae_float_t ae_get_movie_composition_data_duration( const aeMovieCompositionData * _compositionData )
{
    return _compositionData->duration;
}
//////////////////////////////////////////////////////////////////////////
ae_float_t ae_get_movie_composition_data_frame_duration( const aeMovieCompositionData * _compositionData )
{
    return _compositionData->frameDuration;
}
//////////////////////////////////////////////////////////////////////////
ae_uint32_t ae_get_movie_composition_data_frame_count( const aeMovieCompositionData * _compositionData )
{
    return _compositionData->frameCount;
}
//////////////////////////////////////////////////////////////////////////
void ae_get_movie_composition_data_loop_segment( const aeMovieCompositionData * _compositionData, ae_float_t * _in, ae_float_t * _out )
{
    *_in = _compositionData->loop_segment[0];
    *_out = _compositionData->loop_segment[1];
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
ae_uint32_t ae_get_composition_data_event_count( const aeMovieCompositionData * _compositionData )
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
            count += ae_get_composition_data_event_count( it_layer->sub_composition_data );
        }
    }

    return count;
}
//////////////////////////////////////////////////////////////////////////
static ae_bool_t __ae_get_composition_data_event_name( const aeMovieCompositionData * _compositionData, ae_uint32_t * _iterator, ae_uint32_t _index, const ae_char_t ** _name )
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
const ae_char_t * ae_get_composition_data_event_name( const aeMovieCompositionData * _compositionData, ae_uint32_t _index )
{
    ae_uint32_t iterator = 0;
    const ae_char_t * name;
    if( __ae_get_composition_data_event_name( _compositionData, &iterator, _index, &name ) == AE_FALSE )
    {
        return AE_NULL;
    }

    return name;
}
//////////////////////////////////////////////////////////////////////////