#include "em_movieplayer.h"

#include "movie/movie.h"

#include <GLES2/gl2.h>

#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "em_typedef.h"
#include "em_memory.h"
#include "em_math.h"
#include "em_opengles.h"

//////////////////////////////////////////////////////////////////////////
typedef struct em_blend_render_vertex_t
{
    float position[3];
    uint32_t color;
    float uv[2];
} em_blend_render_vertex_t;
//////////////////////////////////////////////////////////////////////////
typedef struct em_blend_shader_t
{
    GLuint program_id;

    GLint positionLocation;
    GLint colorLocation;
    GLint texcoordLocation;
    GLint mvpMatrixLocation;
    GLint tex0Location;
} em_blend_shader_t;
//////////////////////////////////////////////////////////////////////////
typedef struct em_track_matte_render_vertex_t
{
    float position[3];
    uint32_t color;
    float uv0[2];
    float uv1[2];
} em_track_matte_render_vertex_t;
//////////////////////////////////////////////////////////////////////////
typedef struct em_track_matte_shader_t
{
    GLuint program_id;

    GLint positionLocation;
    GLint colorLocation;
    GLint texcoord0Location;
    GLint texcoord1Location;
    GLint mvpMatrixLocation;
    GLint tex0Location;
    GLint tex1Location;
} em_track_matte_shader_t;
//////////////////////////////////////////////////////////////////////////
typedef struct em_player_t
{
    aeMovieInstance * instance;

    uint32_t width;
    uint32_t height;

    uint32_t ud;

    em_blend_shader_t * blend_shader;
    em_track_matte_shader_t * track_matte_shader;

    em_blend_render_vertex_t blend_vertices[1024];
    em_track_matte_render_vertex_t track_matte_vertices[1024];

    float wm[16];

} em_player_t;
//////////////////////////////////////////////////////////////////////////
static void * __instance_alloc( void * _data, size_t _size )
{
    (void)_data;

    return EM_MALLOC( _size );
}
//////////////////////////////////////////////////////////////////////////
static void * __instance_alloc_n( void * _data, size_t _size, size_t _count )
{
    (void)_data;

    size_t total = _size * _count;

    return EM_MALLOC( total );
}
//////////////////////////////////////////////////////////////////////////
static void __instance_free( void * _data, const void * _ptr )
{
    (void)_data;

    EM_FREE( (void *)_ptr );
}
//////////////////////////////////////////////////////////////////////////
static void __instance_free_n( void * _data, const void * _ptr )
{
    (void)_data;

    EM_FREE( (void *)_ptr );
}
//////////////////////////////////////////////////////////////////////////
static void __instance_logerror( void * _data, aeMovieErrorCode _code, const char * _format, ... )
{
    (void)_data;
    (void)_code;
    (void)_format;

    va_list argList;

    va_start( argList, _format );
    char msg[4096];
    vsprintf( msg, _format, argList );
    va_end( argList );

    switch( _code )
    {
    case AE_ERROR_INFO:
    case AE_ERROR_MEMORY:
    case AE_ERROR_STREAM:
        {            
        }break;
    case AE_ERROR_WARNING:
    case AE_ERROR_UNSUPPORT:
        {
            emscripten_log( EM_LOG_WARN, "libmovie: %s"
                , msg 
            );
        }break;
    case AE_ERROR_ERROR:
    case AE_ERROR_INTERNAL:
    case AE_ERROR_CRITICAL:
        {
            emscripten_log( EM_LOG_WARN, "libmovie: %s"
                , msg
            );
        }break;
    }
}
//////////////////////////////////////////////////////////////////////////
static em_blend_shader_t * __make_blend_shader()
{
    const char * vertex_shader_source =
        "uniform highp mat4 g_mvpMatrix;\n"
        "attribute highp vec4 a_position;\n"
        "attribute lowp vec4 a_color;\n"
        "attribute mediump vec2 a_texcoord;\n"
        "varying lowp vec4 v_color;\n"
        "varying mediump vec2 v_texcoord;\n"
        "void main( void )\n"
        "{\n"
        "   gl_Position = g_mvpMatrix * a_position;\n"
        "   v_color = a_color;\n"
        "   v_texcoord = a_texcoord;\n"
        "}\n";

    const char * fragment_shader_source =
        "uniform sampler2D g_tex0;\n"
        "varying lowp vec4 v_color;\n"
        "varying mediump vec2 v_texcoord;\n"
        "void main( void ) {\n"
        "mediump vec4 c = texture2D( g_tex0, v_texcoord );\n"
        "gl_FragColor = v_color * c;\n"
        "}\n";

    GLuint program_id = __make_opengl_program( "__blend__", 100, vertex_shader_source, fragment_shader_source );

    if( program_id == 0U )
    {
        return em_nullptr;
    }

    int positionLocation;
    GLCALLR( positionLocation, glGetAttribLocation, (program_id, "a_position") );

    if( positionLocation == -1 )
    {
        emscripten_log( EM_LOG_ERROR, "opengl invalid attrib a_position '%d'\n"
            , positionLocation
        );

        return em_nullptr;
    }

    int colorLocation;
    GLCALLR( colorLocation, glGetAttribLocation, (program_id, "a_color") );

    if( colorLocation == -1 )
    {
        emscripten_log( EM_LOG_ERROR, "opengl invalid attrib a_color '%d'\n"
            , colorLocation
        );

        return em_nullptr;
    }

    int texcoordLocation;
    GLCALLR( texcoordLocation, glGetAttribLocation, (program_id, "a_texcoord") );

    if( texcoordLocation == -1 )
    {
        emscripten_log( EM_LOG_ERROR, "opengl invalid attrib a_texcoord '%d'\n"
            , texcoordLocation
        );

        return em_nullptr;
    }

    int mvpMatrixLocation;
    GLCALLR( mvpMatrixLocation, glGetUniformLocation, (program_id, "g_mvpMatrix") );

    if( mvpMatrixLocation == -1 )
    {
        emscripten_log( EM_LOG_ERROR, "opengl invalid uniform g_mvpMatrix '%d'\n"
            , mvpMatrixLocation
        );

        return em_nullptr;
    }

    int tex0Location;
    GLCALLR( tex0Location, glGetUniformLocation, (program_id, "g_tex0") );

    if( tex0Location == -1 )
    {
        emscripten_log( EM_LOG_ERROR, "opengl uniform g_tex0 '%d'\n"
            , tex0Location
        );

        return em_nullptr;
    }

    em_blend_shader_t * blend_shader = EM_NEW( em_blend_shader_t );

    blend_shader->program_id = program_id;
    blend_shader->positionLocation = positionLocation;
    blend_shader->colorLocation = colorLocation;
    blend_shader->texcoordLocation = texcoordLocation;
    blend_shader->mvpMatrixLocation = mvpMatrixLocation;
    blend_shader->tex0Location = tex0Location;

    return blend_shader;
}
//////////////////////////////////////////////////////////////////////////
static em_track_matte_shader_t * __make_track_matte_shader()
{
    const char * vertex_shader_source =
        "uniform highp mat4 g_mvpMatrix;\n"
        "attribute highp vec4 a_position;\n"
        "attribute lowp vec4 a_color;\n"
        "attribute mediump vec2 a_texcoord0;\n"
        "attribute mediump vec2 a_texcoord1;\n"
        "varying lowp vec4 v_color;\n"
        "varying mediump vec2 v_texcoord0;\n"
        "varying mediump vec2 v_texcoord1;\n"
        "void main( void )\n"
        "{\n"
        "   gl_Position = g_mvpMatrix * a_position;\n"
        "   v_color = a_color;\n"
        "   v_texcoord0 = a_texcoord0;\n"
        "   v_texcoord1 = a_texcoord1;\n"
        "}\n";

    const char * fragment_shader_source =
        "uniform sampler2D g_tex0;\n"
        "uniform sampler2D g_tex1;\n"
        "varying lowp vec4 v_color;\n"
        "varying mediump vec2 v_texcoord0;\n"
        "varying mediump vec2 v_texcoord1;\n"
        "void main( void ) {\n"
        "mediump vec4 c0 = texture2D( g_tex0, v_texcoord0 );\n"
        "mediump vec4 c1 = texture2D( g_tex1, v_texcoord1 );\n"
        "gl_FragColor = v_color * c0 * vec4(1.0, 1.0, 1.0, c1.a);\n"
        "}\n";

    GLuint program_id = __make_opengl_program( "__track_matte__", 100, vertex_shader_source, fragment_shader_source );

    if( program_id == 0U )
    {
        return em_nullptr;
    }

    int positionLocation;
    GLCALLR( positionLocation, glGetAttribLocation, (program_id, "a_position") );

    if( positionLocation == -1 )
    {
        emscripten_log( EM_LOG_ERROR, "opengl invalid attrib a_position '%d'\n"
            , positionLocation
        );

        return em_nullptr;
    }

    int colorLocation;
    GLCALLR( colorLocation, glGetAttribLocation, (program_id, "a_color") );

    if( colorLocation == -1 )
    {
        emscripten_log( EM_LOG_ERROR, "opengl invalid attrib a_color '%d'\n"
            , colorLocation
        );

        return em_nullptr;
    }

    int texcoord0Location;
    GLCALLR( texcoord0Location, glGetAttribLocation, (program_id, "a_texcoord0") );

    if( texcoord0Location == -1 )
    {
        emscripten_log( EM_LOG_ERROR, "opengl attrib a_texcoord0 '%d'\n"
            , texcoord0Location
        );

        return em_nullptr;
    }

    int texcoord1Location;
    GLCALLR( texcoord1Location, glGetAttribLocation, (program_id, "a_texcoord1") );

    if( texcoord1Location == -1 )
    {
        emscripten_log( EM_LOG_ERROR, "opengl invalid attrib a_texcoord1 '%d'\n"
            , texcoord1Location
        );

        return em_nullptr;
    }

    int mvpMatrixLocation;
    GLCALLR( mvpMatrixLocation, glGetUniformLocation, (program_id, "g_mvpMatrix") );

    if( mvpMatrixLocation == -1 )
    {
        emscripten_log( EM_LOG_ERROR, "opengl invalid uniform g_mvpMatrix '%d'\n"
            , mvpMatrixLocation
        );

        return em_nullptr;
    }

    int tex0Location;
    GLCALLR( tex0Location, glGetUniformLocation, (program_id, "g_tex0") );

    if( tex0Location == -1 )
    {
        emscripten_log( EM_LOG_ERROR, "opengl invalid uniform g_tex0 '%d'\n"
            , tex0Location
        );

        return em_nullptr;
    }

    int tex1Location;
    GLCALLR( tex1Location, glGetUniformLocation, (program_id, "g_tex1") );

    if( tex1Location == -1 )
    {
        emscripten_log( EM_LOG_ERROR, "opengl invalid uniform g_tex1 '%d'\n"
            , tex1Location
        );

        return em_nullptr;
    }

    em_track_matte_shader_t * track_matte_shader = EM_NEW( em_track_matte_shader_t );

    track_matte_shader->program_id = program_id;
    track_matte_shader->positionLocation = positionLocation;
    track_matte_shader->colorLocation = colorLocation;
    track_matte_shader->texcoord0Location = texcoord0Location;
    track_matte_shader->texcoord1Location = texcoord1Location;
    track_matte_shader->mvpMatrixLocation = mvpMatrixLocation;
    track_matte_shader->tex0Location = tex0Location;
    track_matte_shader->tex1Location = tex1Location;

    return track_matte_shader;
}
//////////////////////////////////////////////////////////////////////////
em_player_handle_t em_create_player( const char * _hashkey, uint32_t _width, uint32_t _height, uint32_t _ud )
{
    em_player_t * player = EM_NEW( em_player_t );

    aeMovieInstance * ae_instance = ae_create_movie_instance( _hashkey
        , &__instance_alloc
        , &__instance_alloc_n
        , &__instance_free
        , &__instance_free_n
        , AE_NULL
        , &__instance_logerror
        , player );

    player->instance = ae_instance;

    player->width = _width;
    player->height = _height;
    player->ud = _ud;

    em_blend_shader_t * blend_shader = __make_blend_shader();

    if( blend_shader == em_nullptr )
    {
        return em_nullptr;
    }

    player->blend_shader = blend_shader;

    em_track_matte_shader_t * track_matte_shader = __make_track_matte_shader();

    if( track_matte_shader == em_nullptr )
    {
        return em_nullptr;
    }

    player->track_matte_shader = track_matte_shader;

    __identity_m4( player->wm );

    return player;
}
//////////////////////////////////////////////////////////////////////////
void em_delete_player( em_player_handle_t _player )
{
    em_player_t * player = (em_player_t *)_player;

    EM_FREE( player->blend_shader );

    ae_delete_movie_instance( player->instance );

    EM_FREE( player );
}
//////////////////////////////////////////////////////////////////////////
static ae_size_t __read_file( ae_voidptr_t _data, ae_voidptr_t _buff, ae_uint32_t _carriage, ae_uint32_t _size )
{
    (void)_carriage;

    FILE * f = (FILE *)_data;

    ae_size_t s = fread( _buff, 1, _size, f );

    return s;
}
//////////////////////////////////////////////////////////////////////////
static void __memory_copy( ae_voidptr_t _data, ae_constvoidptr_t _src, ae_voidptr_t _dst, ae_size_t _size )
{
    (void)_data;
    memcpy( _dst, _src, _size );
}
//////////////////////////////////////////////////////////////////////////
typedef struct em_resource_image_t
{
    GLuint texture_id;
} em_resource_image_t;
//////////////////////////////////////////////////////////////////////////
typedef struct em_resource_sound_t
{
    uint32_t ud;
} em_resource_sound_t;
//////////////////////////////////////////////////////////////////////////
static ae_voidptr_t __ae_movie_data_resource_provider( const aeMovieResource * _resource, ae_voidptr_t _ud )
{
    em_player_t * em_player = (em_player_t *)_ud;

    switch( _resource->type )
    {
    case AE_MOVIE_RESOURCE_IMAGE:
        {
            const aeMovieResourceImage * r = (const aeMovieResourceImage *)_resource;

            em_resource_image_t * resource_image = EM_NEW( em_resource_image_t );

            GLuint texture_id = __make_opengl_texture();

            EM_ASM(
            {
                em_player_resource_image_provider( $0, $1, Module.Pointer_stringify( $2 ), $3, $4 );
            }, em_player->ud, texture_id, r->path, r->codec, r->premultiplied );
            
            resource_image->texture_id = texture_id;

            return resource_image;            
        }break;
    case AE_MOVIE_RESOURCE_SEQUENCE:
        {   
        }break;
    case AE_MOVIE_RESOURCE_VIDEO:
        {
            emscripten_log( EM_LOG_ERROR, "Unsuported resource type: video.\n"
            );
            
        }break;
    case AE_MOVIE_RESOURCE_SOUND:
        {
            const aeMovieResourceSound * r = (const aeMovieResourceSound *)_resource;

            uint32_t ud = EM_ASM_INT(
            {
                return em_player_resource_sound_provider( $0, Module.Pointer_stringify( $1 ), $2, $3 );
            }, em_player->ud, r->path, r->codec, r->duration );

            em_resource_sound_t * resource = EM_NEW( em_resource_sound_t );

            resource->ud = ud;

            return resource;            
        }break;
    case AE_MOVIE_RESOURCE_SLOT:
        {
            const aeMovieResourceSlot * r = (const aeMovieResourceSlot *)_resource;
                        
            //TODO
        }break;
    default:
        {
            emscripten_log( EM_LOG_ERROR, "Unsuport resource type: other (%i).\n"
                , _resource->type 
            );

            break;
        }
    }

    return AE_NULL;
}
//////////////////////////////////////////////////////////////////////////
static void __ae_movie_data_resource_deleter( aeMovieResourceTypeEnum _type, ae_voidptr_t * _data, ae_voidptr_t _ud )
{
    em_player_t * em_player = (em_player_t *)_ud;

    switch( _type )
    {
    case AE_MOVIE_RESOURCE_IMAGE:
        {
            em_resource_image_t * image_data = (em_resource_image_t *)_data;

            glDeleteTextures( 1, &image_data->texture_id );

            EM_FREE( _data );

            break;
        }
    case AE_MOVIE_RESOURCE_SEQUENCE:
        {
            break;
        }
    case AE_MOVIE_RESOURCE_VIDEO:
        {
            break;
        }
    case AE_MOVIE_RESOURCE_SOUND:
        {
            em_resource_sound_t * resource = (em_resource_sound_t *)_data;
            
            EM_ASM(
            {
                em_player_resource_sound_deleter( $0, $1 );
            }, em_player->ud, resource->ud );
            
            EM_FREE( _data );

            break;
        }
    case AE_MOVIE_RESOURCE_SLOT:
        {
            break;
        }
    default:
        {
            break;
        }
    }
}
//////////////////////////////////////////////////////////////////////////
em_movie_data_handle_t em_create_movie_data( em_player_handle_t _player, const uint8_t * _data )
{    
    em_player_t * em_player = (em_player_t *)_player;

    aeMovieData * movie_data = ae_create_movie_data( em_player->instance, &__ae_movie_data_resource_provider, &__ae_movie_data_resource_deleter, em_player );

    if( movie_data == em_nullptr )
    {
        emscripten_log( EM_LOG_ERROR, "invalid create movie data"            
        );

        return em_nullptr;
    }

    aeMovieStream * stream = ae_create_movie_stream_memory( em_player->instance, _data, &__memory_copy, AE_NULL );

    ae_result_t result_load = ae_load_movie_data( movie_data, stream );

    if( result_load != AE_MOVIE_SUCCESSFUL )
    {
        emscripten_log( EM_LOG_ERROR, "invalid load movie data 'result %d'"
            , result_load 
        );

        return em_nullptr;
    }

    ae_delete_movie_stream( stream );
    
    return movie_data;
}
//////////////////////////////////////////////////////////////////////////
void em_delete_movie_data( em_movie_data_handle_t _movieData )
{
    aeMovieData * movie_data = (aeMovieData *)_movieData;

    ae_delete_movie_data( movie_data );
}
//////////////////////////////////////////////////////////////////////////
typedef struct em_node_track_matte_t
{
    em_resource_image_t * base_image;
    em_resource_image_t * track_matte_image;
} em_node_track_matte_t;
//////////////////////////////////////////////////////////////////////////
typedef struct em_node_sound_t
{
    em_resource_sound_t * resource;
} em_node_sound_t;
//////////////////////////////////////////////////////////////////////////
static ae_voidptr_t __ae_movie_composition_node_provider( const aeMovieNodeProviderCallbackData * _callbackData, void * _data )
{
    (void)_data;

    const aeMovieLayerData * layer = _callbackData->layer;

    ae_bool_t is_track_matte = ae_is_movie_layer_data_track_mate( layer );

    if( is_track_matte == AE_TRUE )
    {
        return AE_NULL;
    }

    aeMovieLayerTypeEnum type = ae_get_movie_layer_data_type( layer );

    if( _callbackData->track_matte_layer != AE_NULL )
    {
        switch( type )
        {
        case AE_MOVIE_LAYER_TYPE_IMAGE:
            {
                em_node_track_matte_t * node_track_matte = EM_NEW( em_node_track_matte_t );

                em_resource_image_t * base_image = (em_resource_image_t *)ae_get_movie_layer_data_resource_data( _callbackData->layer );
                em_resource_image_t * track_matte_image = (em_resource_image_t *)ae_get_movie_layer_data_resource_data( _callbackData->track_matte_layer );

                node_track_matte->base_image = base_image;
                node_track_matte->track_matte_image = track_matte_image;

                return node_track_matte;
            }break;
        default:
            {
            }break;
        }
    }
    else
    {
        switch( type )
        {
        case AE_MOVIE_LAYER_TYPE_VIDEO:
            {
                //Empty

                return AE_NULL;
            }break;
        case AE_MOVIE_LAYER_TYPE_SOUND:
            {
                em_node_sound_t * node_sound = EM_NEW( em_node_sound_t );

                ae_voidptr_t resource_data = ae_get_movie_layer_data_resource_data( layer );

                node_sound->resource = (em_resource_sound_t *)resource_data;

                return node_sound;
            }break;
        default:
            {
            }break;
        }
    }

    return AE_NULL;
}
//////////////////////////////////////////////////////////////////////////
static void __ae_movie_composition_node_deleter( const aeMovieNodeDeleterCallbackData * _callbackData, void * _data )
{
    (void)_data;

    const aeMovieLayerData * layer = _callbackData->layer;

    ae_bool_t is_track_matte = ae_is_movie_layer_data_track_mate( layer );

    if( is_track_matte == AE_TRUE )
    {
        return;
    }

    aeMovieLayerTypeEnum type = ae_get_movie_layer_data_type( layer );

    if( _callbackData->track_matte_layer != AE_NULL )
    {
        switch( type )
        {
        case AE_MOVIE_LAYER_TYPE_IMAGE:
            {                
                em_node_track_matte_t * node_track_matte = (em_node_track_matte_t *)_callbackData->element;

                EM_DELETE( node_track_matte );
            }break;
        default:
            {
            }break;
        }
    }
    else
    {
        switch( type )
        {
        case AE_MOVIE_LAYER_TYPE_VIDEO:
            {
            }break;
        case AE_MOVIE_LAYER_TYPE_SOUND:
            {
                em_node_sound_t * node_sound = (em_node_sound_t *)_callbackData->element;

                EM_DELETE( node_sound );
            }break;
        default:
            {
            }break;
        }
    }
}
//////////////////////////////////////////////////////////////////////////
static void __ae_movie_composition_node_update( const aeMovieNodeUpdateCallbackData * _callbackData, ae_voidptr_t _data )
{
    em_player_t * em_player = (em_player_t *)_data;

    switch( _callbackData->state )
    {
    case AE_MOVIE_NODE_UPDATE_UPDATE:
        {
            //Empty
        }break;
    case AE_MOVIE_NODE_UPDATE_BEGIN:
        {
            switch( _callbackData->type )
            {
            case AE_MOVIE_LAYER_TYPE_VIDEO:
                {
                    //Empty
                }break;
            case AE_MOVIE_LAYER_TYPE_SOUND:
                {
                    em_node_sound_t * node_sound = (em_node_sound_t *)_callbackData->element;

                    em_resource_sound_t * resource_sound = node_sound->resource;

                    EM_ASM(
                    {
                        em_player_node_sound_play( $0, $1, $2 );
                    }, em_player->ud, resource_sound->ud, _callbackData->offset );
                    
                }break;
            }
        }break;
    case AE_MOVIE_NODE_UPDATE_END:
        {
            switch( _callbackData->type )
            {
            case AE_MOVIE_LAYER_TYPE_VIDEO:
                {
                    //Empty
                }break;
            case AE_MOVIE_LAYER_TYPE_SOUND:
                {
                    em_node_sound_t * node_sound = (em_node_sound_t *)_callbackData->element;

                    em_resource_sound_t * resource_sound = node_sound->resource;

                    EM_ASM(
                    {
                        em_player_node_sound_stop( $0, $1 );
                    }, em_player->ud, resource_sound->ud );

                }break;
            }
        }break;
    case AE_MOVIE_NODE_UPDATE_PAUSE:
        {
            switch( _callbackData->type )
            {
            case AE_MOVIE_LAYER_TYPE_VIDEO:
                {
                    //Empty
                }break;
            case AE_MOVIE_LAYER_TYPE_SOUND:
                {
                    em_node_sound_t * node_sound = (em_node_sound_t *)_callbackData->element;

                    em_resource_sound_t * resource_sound = node_sound->resource;

                    EM_ASM(
                    {
                        em_player_node_sound_pause( $0, $1 );
                    }, em_player->ud, resource_sound->ud );

                }break;
            }
        }break;
    case AE_MOVIE_NODE_UPDATE_RESUME:
        {
            switch( _callbackData->type )
            {
            case AE_MOVIE_LAYER_TYPE_VIDEO:
                {
                    //Empty
                }break;
            case AE_MOVIE_LAYER_TYPE_SOUND:
                {
                    em_node_sound_t * node_sound = (em_node_sound_t *)_callbackData->element;

                    em_resource_sound_t * resource_sound = node_sound->resource;

                    EM_ASM(
                    {
                        em_player_node_sound_resume( $0, $1 );
                    }, em_player->ud, resource_sound->ud );

                }break;
            }
        }break;
    }
}
//////////////////////////////////////////////////////////////////////////
typedef struct ae_camera_t
{
    float view[16];
    float projection[16];
    float width;
    float height;
} ae_camera_t;
//////////////////////////////////////////////////////////////////////////
static void * __ae_movie_callback_camera_provider( const aeMovieCameraProviderCallbackData * _callbackData, void * _data )
{
    (void)_data;

    ae_camera_t * camera = EM_NEW( ae_camera_t );

    __make_lookat_m4( camera->view, _callbackData->position, _callbackData->target );
    __make_projection_fov_m4( camera->projection, _callbackData->width, _callbackData->height, _callbackData->fov );

    camera->width = _callbackData->width;
    camera->height = _callbackData->height;

    return camera;
}
//////////////////////////////////////////////////////////////////////////
static void __ae_movie_callback_camera_deleter( const aeMovieCameraDeleterCallbackData * _callbackData, ae_voidptr_t _data )
{
    (void)_data;

    ae_camera_t * camera = (ae_camera_t *)_callbackData->element;

    EM_FREE( camera );
}
//////////////////////////////////////////////////////////////////////////
static void __ae_movie_callback_camera_update( const aeMovieCameraUpdateCallbackData * _callbackData, void * _data )
{
    (void)_data;

    ae_camera_t * camera = (ae_camera_t *)_callbackData->element;

    __make_lookat_m4( camera->view, _callbackData->position, _callbackData->target );
}
//////////////////////////////////////////////////////////////////////////
typedef struct em_parameter_color_t
{
    GLfloat r;
    GLfloat g;
    GLfloat b;
} em_parameter_color_t;
//////////////////////////////////////////////////////////////////////////
typedef struct em_custom_shader_t
{
    GLuint program_id;

    uint8_t parameter_count;
    int8_t parameter_types[32];
    GLint parameter_locations[32];

    GLfloat parameter_values[32];
    em_parameter_color_t parameter_colors[32];

    GLint positionLocation;
    GLint colorLocation;
    GLint texcoordLocation;

    GLint mvpMatrixLocation;
    GLint tex0Location;
} em_custom_shader_t;
//////////////////////////////////////////////////////////////////////////
static ae_voidptr_t __ae_movie_callback_shader_provider( const aeMovieShaderProviderCallbackData * _callbackData, ae_voidptr_t _data )
{
    (void)_data;

    em_custom_shader_t * shader = EM_NEW( em_custom_shader_t );

    GLuint program_id = __make_opengl_program( _callbackData->name, _callbackData->version, _callbackData->shader_vertex, _callbackData->shader_fragment );

    if( program_id == 0U )
    {
        return em_nullptr;
    }

    shader->program_id = program_id;

    int positionLocation;
    GLCALLR( positionLocation, glGetAttribLocation, (program_id, "a_position") );

    if( positionLocation == -1 )
    {
        emscripten_log( EM_LOG_ERROR, "shader '%s' invalid attrib inVert '%d'\n"
            , _callbackData->name
            , positionLocation
        );

        return em_nullptr;
    }

    shader->positionLocation = positionLocation;

    int colorLocation;
    GLCALLR( colorLocation, glGetAttribLocation, (program_id, "a_color") );

    if( colorLocation == -1 )
    {
        emscripten_log( EM_LOG_ERROR, "shader '%s' invalid attrib inCol '%d'\n"
            , _callbackData->name
            , colorLocation
        );

        return em_nullptr;
    }

    shader->colorLocation = colorLocation;

    int texcoordLocation;
    GLCALLR( texcoordLocation, glGetAttribLocation, (program_id, "a_texcoord") );

    if( texcoordLocation == -1 )
    {
        emscripten_log( EM_LOG_ERROR, "shader '%s' invalid attrib inUV '%d'\n"
            , _callbackData->name
            , texcoordLocation
        );

        return em_nullptr;
    }

    shader->texcoordLocation = texcoordLocation;

    shader->parameter_count = _callbackData->parameter_count;

    for( uint32_t i = 0; i != _callbackData->parameter_count; ++i )
    {
        const char * parameter_name = _callbackData->parameter_names[i];
        const char * parameter_uniform = _callbackData->parameter_uniforms[i];
        uint8_t parameter_type = _callbackData->parameter_types[i];

        shader->parameter_types[i] = parameter_type;

        GLint parameter_location;
        GLCALLR( parameter_location, glGetUniformLocation, (program_id, parameter_uniform) );

        if( parameter_location == -1 )
        {
            emscripten_log( EM_LOG_ERROR, "shader '%s' invalid attrib '%s' uniform '%s' location '%d'\n"
                , _callbackData->name
                , parameter_name
                , parameter_uniform
                , parameter_location
            );

            return em_nullptr;
        }

        shader->parameter_locations[i] = parameter_location;
    }

    int mvpMatrixLocation;
    GLCALLR( mvpMatrixLocation, glGetUniformLocation, (program_id, "g_mvpMatrix") );

    if( mvpMatrixLocation == -1 )
    {
        emscripten_log( EM_LOG_ERROR, "shader '%s' invalid uniform mvpMatrix '%d'\n"
            , _callbackData->name
            , mvpMatrixLocation
        );

        return em_nullptr;
    }

    shader->mvpMatrixLocation = mvpMatrixLocation;

    int tex0Location;
    GLCALLR( tex0Location, glGetUniformLocation, (program_id, "g_tex0") );

    if( tex0Location == -1 )
    {
        emscripten_log( EM_LOG_ERROR, "shader '%s' invalid uniform tex0 '%d'\n"
            , _callbackData->name
            , tex0Location
        );

        return em_nullptr;
    }

    shader->tex0Location = tex0Location;

    return shader;
}
//////////////////////////////////////////////////////////////////////////
static void __ae_movie_callback_shader_property_update( const aeMovieShaderPropertyUpdateCallbackData * _callbackData, ae_voidptr_t _data )
{
    (void)_data;

    em_custom_shader_t * shader = (em_custom_shader_t *)_callbackData->element;

    uint8_t index = _callbackData->index;

    GLint location = shader->parameter_locations[index];

    switch( _callbackData->type )
    {
    case 3:
        {
            shader->parameter_values[index] = _callbackData->value;
        }break;
    case 5:
        {
            em_parameter_color_t c;
            c.r = _callbackData->color_r;
            c.g = _callbackData->color_g;
            c.b = _callbackData->color_b;

            shader->parameter_colors[index] = c;
        }
    }
}
//////////////////////////////////////////////////////////////////////////
static void __ae_movie_callback_shader_deleter( const aeMovieShaderDeleterCallbackData * _callbackData, ae_voidptr_t _data )
{
    (void)_data;

    EM_FREE( _callbackData->element );
}
//////////////////////////////////////////////////////////////////////////
typedef struct em_track_matte_t
{
    float matrix[16];
    aeMovieRenderMesh mesh;
} em_track_matte_t;
//////////////////////////////////////////////////////////////////////////
static void * __ae_movie_callback_track_matte_provider( const aeMovieTrackMatteProviderCallbackData * _callbackData, void * _data )
{
    (void)_data;

    em_track_matte_t * track_matte = EM_NEW( em_track_matte_t );

    __copy_m4( track_matte->matrix, _callbackData->matrix );
    track_matte->mesh = *_callbackData->mesh;

    return track_matte;
}
//////////////////////////////////////////////////////////////////////////
static void __ae_movie_callback_track_matte_update( const aeMovieTrackMatteUpdateCallbackData * _callbackData, void * _data )
{
    (void)_data;

    em_track_matte_t * track_matte = (em_track_matte_t *)_callbackData->track_matte_data;

    switch( _callbackData->state )
    {
    case AE_MOVIE_NODE_UPDATE_BEGIN:
        {
            __copy_m4( track_matte->matrix, _callbackData->matrix );
            track_matte->mesh = *_callbackData->mesh;
        }break;
    case AE_MOVIE_NODE_UPDATE_UPDATE:
        {
            __copy_m4( track_matte->matrix, _callbackData->matrix );
            track_matte->mesh = *_callbackData->mesh;
        }break;
    }
}
//////////////////////////////////////////////////////////////////////////
static void __ae_movie_callback_track_matte_deleter( const aeMovieTrackMatteDeleterCallbackData * _callbackData, void * _data )
{
    (void)_data;

    em_track_matte_t * track_matte = (em_track_matte_t *)_callbackData->element;

    EM_FREE( track_matte );
}
//////////////////////////////////////////////////////////////////////////
em_movie_composition_handle_t em_create_movie_composition( em_player_handle_t _player, em_movie_data_handle_t _movieData, const char * _name )
{
    em_player_t * em_player = (em_player_t *)_player;
    aeMovieData * ae_movie_data = (aeMovieData *)_movieData;

    const ae_char_t * movieName = ae_get_movie_name( ae_movie_data );

    const aeMovieCompositionData * movieCompositionData = ae_get_movie_composition_data( ae_movie_data, _name );

    if( movieCompositionData == AE_NULL )
    {
        emscripten_log( EM_LOG_ERROR, "error get movie '%s' composition data '%s'"
            , movieName
            , _name
        );

        return AE_NULL;
    }

    aeMovieCompositionProviders providers;
    ae_initialize_movie_composition_providers( &providers );

    providers.node_provider = &__ae_movie_composition_node_provider;
    providers.node_deleter = &__ae_movie_composition_node_deleter;
    providers.node_update = &__ae_movie_composition_node_update;

    providers.camera_provider = &__ae_movie_callback_camera_provider;
    providers.camera_deleter = &__ae_movie_callback_camera_deleter;
    providers.camera_update = &__ae_movie_callback_camera_update;

    providers.shader_provider = &__ae_movie_callback_shader_provider;
    providers.shader_deleter = &__ae_movie_callback_shader_deleter;
    providers.shader_property_update = &__ae_movie_callback_shader_property_update;

    providers.track_matte_provider = &__ae_movie_callback_track_matte_provider;
    providers.track_matte_update = &__ae_movie_callback_track_matte_update;
    providers.track_matte_deleter = &__ae_movie_callback_track_matte_deleter;

    aeMovieComposition * movieComposition = ae_create_movie_composition( ae_movie_data, movieCompositionData, AE_TRUE, &providers, em_player );

    if( movieComposition == AE_NULL )
    {
        emscripten_log( EM_LOG_ERROR, "error create movie '%s' composition '%s'"
            , movieName
            , _name
        );

        return AE_NULL;
    }

    return movieComposition;
}
//////////////////////////////////////////////////////////////////////////
void em_delete_movie_composition( em_movie_composition_handle_t _movieComposition )
{
    const aeMovieComposition * ae_movie_composition = (const aeMovieComposition *)_movieComposition;

    ae_delete_movie_composition( ae_movie_composition );
}
//////////////////////////////////////////////////////////////////////////
void em_set_movie_composition_loop( em_movie_composition_handle_t _movieComposition, unsigned int _loop )
{
    const aeMovieComposition * ae_movie_composition = (const aeMovieComposition *)_movieComposition;

    ae_set_movie_composition_loop( ae_movie_composition, _loop );
}
//////////////////////////////////////////////////////////////////////////
void em_play_movie_composition( em_movie_composition_handle_t _movieComposition, float _time )
{
    aeMovieComposition * ae_movie_composition = (aeMovieComposition *)_movieComposition;

    ae_play_movie_composition( ae_movie_composition, _time );
}
//////////////////////////////////////////////////////////////////////////
void em_set_movie_wm( em_player_handle_t _player, float _px, float _py, float _ox, float _oy, float _sx, float _sy, float _angle )
{
    em_player_t * player = (em_player_t *)_player;

    float matrix_base[16];
    matrix_base[0 * 4 + 0] = _sx;
    matrix_base[0 * 4 + 1] = 0.f;
    matrix_base[0 * 4 + 2] = 0.f;
    matrix_base[0 * 4 + 3] = 0.f;

    matrix_base[1 * 4 + 0] = 0.f;
    matrix_base[1 * 4 + 1] = _sy;
    matrix_base[1 * 4 + 2] = 0.f;
    matrix_base[1 * 4 + 3] = 0.f;

    matrix_base[2 * 4 + 0] = 0.f;
    matrix_base[2 * 4 + 1] = 0.f;
    matrix_base[2 * 4 + 2] = 1.f;
    matrix_base[2 * 4 + 3] = 0.f;

    matrix_base[3 * 4 + 0] = -_ox * _sx;
    matrix_base[3 * 4 + 1] = -_oy * _sy;
    matrix_base[3 * 4 + 2] = 0.f;
    matrix_base[3 * 4 + 3] = 1.f;

    if( _angle != 0.f )
    {
        float cosa = cosf( _angle );
        float sina = sinf( _angle );

        float matrix_rotate[16];
        matrix_rotate[0 * 4 + 0] = cosa;
        matrix_rotate[0 * 4 + 1] = -sina;
        matrix_rotate[0 * 4 + 2] = 0.f;
        matrix_rotate[0 * 4 + 3] = 0.f;

        matrix_rotate[1 * 4 + 0] = sina;
        matrix_rotate[1 * 4 + 1] = cosa;
        matrix_rotate[1 * 4 + 2] = 0.f;
        matrix_rotate[1 * 4 + 3] = 0.f;

        matrix_rotate[2 * 4 + 0] = 0.f;
        matrix_rotate[2 * 4 + 1] = 0.f;
        matrix_rotate[2 * 4 + 2] = 1.f;
        matrix_rotate[2 * 4 + 3] = 0.f;

        matrix_rotate[3 * 4 + 0] = 0.f;
        matrix_rotate[3 * 4 + 1] = 0.f;
        matrix_rotate[3 * 4 + 2] = 0.f;
        matrix_rotate[3 * 4 + 3] = 1.f;

        __mul_m4_m4( player->wm, matrix_base, matrix_rotate );
    }
    else
    {
        __copy_m4( player->wm, matrix_base );
    }

    player->wm[3 * 4 + 0] += _px;
    player->wm[3 * 4 + 1] += _py;
}
//////////////////////////////////////////////////////////////////////////
void em_update_movie_composition( em_player_handle_t _player, em_movie_composition_handle_t _movieComposition, float _time )
{
    (void)_player;

    aeMovieComposition * ae_movie_composition = (aeMovieComposition *)_movieComposition;

    ae_update_movie_composition( ae_movie_composition, _time );
}
//////////////////////////////////////////////////////////////////////////
void em_render_movie_composition( em_player_handle_t _player, em_movie_composition_handle_t _movieComposition )
{
    em_player_t * player = (em_player_t *)_player;

    const aeMovieComposition * ae_movie_composition = (const aeMovieComposition *)_movieComposition;

    uint32_t mesh_iterator = 0;

    aeMovieRenderMesh mesh;
    while( ae_compute_movie_mesh( ae_movie_composition, &mesh_iterator, &mesh ) == AE_TRUE )
    {
        switch( mesh.blend_mode )
        {
        case AE_MOVIE_BLEND_ADD:
            {
                GLCALL( glBlendFunc, (GL_SRC_ALPHA, GL_ONE) );
            }break;
        case AE_MOVIE_BLEND_NORMAL:
            {
                GLCALL( glBlendFunc, (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
            }break;
        case AE_MOVIE_BLEND_ALPHA_ADD:
        case AE_MOVIE_BLEND_CLASSIC_COLOR_BURN:
        case AE_MOVIE_BLEND_CLASSIC_COLOR_DODGE:
        case AE_MOVIE_BLEND_CLASSIC_DIFFERENCE:
        case AE_MOVIE_BLEND_COLOR:
        case AE_MOVIE_BLEND_COLOR_BURN:
        case AE_MOVIE_BLEND_COLOR_DODGE:
        case AE_MOVIE_BLEND_DANCING_DISSOLVE:
        case AE_MOVIE_BLEND_DARKEN:
        case AE_MOVIE_BLEND_DARKER_COLOR:
        case AE_MOVIE_BLEND_DIFFERENCE:
        case AE_MOVIE_BLEND_DISSOLVE:
        case AE_MOVIE_BLEND_EXCLUSION:
        case AE_MOVIE_BLEND_HARD_LIGHT:
        case AE_MOVIE_BLEND_HARD_MIX:
        case AE_MOVIE_BLEND_HUE:
        case AE_MOVIE_BLEND_LIGHTEN:
        case AE_MOVIE_BLEND_LIGHTER_COLOR:
        case AE_MOVIE_BLEND_LINEAR_BURN:
        case AE_MOVIE_BLEND_LINEAR_DODGE:
        case AE_MOVIE_BLEND_LINEAR_LIGHT:
        case AE_MOVIE_BLEND_LUMINESCENT_PREMUL:
        case AE_MOVIE_BLEND_LUMINOSITY:
        case AE_MOVIE_BLEND_MULTIPLY:
        case AE_MOVIE_BLEND_OVERLAY:
        case AE_MOVIE_BLEND_PIN_LIGHT:
        case AE_MOVIE_BLEND_SATURATION:
        case AE_MOVIE_BLEND_SCREEN:
        case AE_MOVIE_BLEND_SILHOUETE_ALPHA:
        case AE_MOVIE_BLEND_SILHOUETTE_LUMA:
        case AE_MOVIE_BLEND_SOFT_LIGHT:
        case AE_MOVIE_BLEND_STENCIL_ALPHA:
        case AE_MOVIE_BLEND_STENCIL_LUMA:
        case AE_MOVIE_BLEND_VIVID_LIGHT:
            {

            }break;
        }

        if( mesh.track_matte_data == em_nullptr )
        {
            size_t opengl_indices_buffer_size = mesh.indexCount * sizeof( em_render_index_t );
            size_t opengl_vertex_buffer_size = mesh.vertexCount * sizeof( em_blend_render_vertex_t );

            GLuint texture_id = 0U;

            em_blend_render_vertex_t * vertices = player->blend_vertices;
            const ae_uint16_t * indices = mesh.indices;

            switch( mesh.layer_type )
            {
            case AE_MOVIE_LAYER_TYPE_SHAPE:
            case AE_MOVIE_LAYER_TYPE_SOLID:
                {
                    uint32_t color = __make_argb( mesh.r, mesh.g, mesh.b, mesh.a );

                    for( uint32_t index = 0; index != mesh.vertexCount; ++index )
                    {
                        em_blend_render_vertex_t * v = vertices + index;

                        const float * mesh_position = mesh.position[index];
                        __v3_v3_m4( v->position, mesh_position, player->wm );

                        v->color = color;

                        const float * mesh_uv = mesh.uv[index];

                        v->uv[0] = mesh_uv[0];
                        v->uv[1] = mesh_uv[1];
                    }
                }break;
            case AE_MOVIE_LAYER_TYPE_SEQUENCE:
            case AE_MOVIE_LAYER_TYPE_IMAGE:
                {
                    const em_resource_image_t * resource_image = (const em_resource_image_t *)mesh.resource_data;

                    texture_id = resource_image->texture_id;

                    uint32_t color = __make_argb( mesh.r, mesh.g, mesh.b, mesh.a );

                    for( uint32_t index = 0; index != mesh.vertexCount; ++index )
                    {
                        em_blend_render_vertex_t * v = vertices + index;

                        const float * mesh_position = mesh.position[index];
                        __v3_v3_m4( v->position, mesh_position, player->wm );

                        v->color = color;

                        const float * mesh_uv = mesh.uv[index];

                        v->uv[0] = mesh_uv[0];
                        v->uv[1] = mesh_uv[1];
                    }
                }break;
            case AE_MOVIE_LAYER_TYPE_VIDEO:
                {
                }break;
            }

            GLuint opengl_vertex_buffer_id = 0;
            GLCALL( glGenBuffers, (1, &opengl_vertex_buffer_id) );
            GLCALL( glBindBuffer, (GL_ARRAY_BUFFER, opengl_vertex_buffer_id) );
            GLCALL( glBufferData, (GL_ARRAY_BUFFER, opengl_vertex_buffer_size, vertices, GL_STREAM_DRAW) );

            GLint positionLocation;
            GLint colorLocation;
            GLint texcoordLocation;
            GLint mvpMatrixLocation;
            GLint tex0Location;

            GLuint program_id;

            if( mesh.shader_data == AE_NULL )
            {
                em_blend_shader_t * blend_shader = player->blend_shader;

                program_id = blend_shader->program_id;

                GLCALL( glUseProgram, (blend_shader->program_id) );

                positionLocation = blend_shader->positionLocation;
                colorLocation = blend_shader->colorLocation;
                texcoordLocation = blend_shader->texcoordLocation;
                mvpMatrixLocation = blend_shader->mvpMatrixLocation;
                tex0Location = blend_shader->tex0Location;
            }
            else
            {
                em_custom_shader_t * shader = (em_custom_shader_t *)mesh.shader_data;

                program_id = shader->program_id;

                GLCALL( glUseProgram, (shader->program_id) );

                uint8_t parameter_count = shader->parameter_count;
                for( uint8_t i = 0; i != parameter_count; ++i )
                {
                    GLint parameter_location = shader->parameter_locations[i];

                    uint8_t parameter_type = shader->parameter_types[i];

                    switch( parameter_type )
                    {
                    case 3:
                        {
                            float value = shader->parameter_values[i];
                            GLCALL( glUniform1f, (parameter_location, value) );
                        }break;
                    case 5:
                        {
                            em_parameter_color_t * c = shader->parameter_colors + i;
                            GLCALL( glUniform3f, (parameter_location, c->r, c->g, c->b) );
                        }break;
                    }
                }

                positionLocation = shader->positionLocation;
                colorLocation = shader->colorLocation;
                texcoordLocation = shader->texcoordLocation;
                mvpMatrixLocation = shader->mvpMatrixLocation;
                tex0Location = shader->tex0Location;
            }

            GLCALL( glActiveTexture, (GL_TEXTURE0) );

            if( texture_id != 0U )
            {
                GLCALL( glBindTexture, (GL_TEXTURE_2D, texture_id) );

                GLCALL( glUniform1i, (tex0Location, 0) );
            }
            else
            {
                GLCALL( glBindTexture, (GL_TEXTURE_2D, 0U) );
            }

            GLCALL( glEnableVertexAttribArray, (positionLocation) );
            GLCALL( glEnableVertexAttribArray, (colorLocation) );
            GLCALL( glEnableVertexAttribArray, (texcoordLocation) );

            GLCALL( glVertexAttribPointer, (positionLocation, 3, GL_FLOAT, GL_FALSE, sizeof( em_blend_render_vertex_t ), (const GLvoid *)offsetof( em_blend_render_vertex_t, position )) );
            GLCALL( glVertexAttribPointer, (colorLocation, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof( em_blend_render_vertex_t ), (const GLvoid *)offsetof( em_blend_render_vertex_t, color )) );
            GLCALL( glVertexAttribPointer, (texcoordLocation, 2, GL_FLOAT, GL_FALSE, sizeof( em_blend_render_vertex_t ), (const GLvoid *)offsetof( em_blend_render_vertex_t, uv )) );

            if( mesh.camera_data == AE_NULL )
            {
                const aeMovieCompositionData * ae_movie_composition_data = ae_get_movie_composition_composition_data( ae_movie_composition );

                float viewMatrix[16];
                __identity_m4( viewMatrix );

                float composition_width = ae_get_movie_composition_data_width( ae_movie_composition_data );
                float composition_height = ae_get_movie_composition_data_height( ae_movie_composition_data );
                
                float width = player->width;
                float height = player->height;

                float projectionMatrix[16];
                __make_orthogonal_m4( projectionMatrix, width, height );

                float projectionViewMatrix[16];
                __mul_m4_m4( projectionViewMatrix, projectionMatrix, viewMatrix );

                GLCALL( glUniformMatrix4fv, (mvpMatrixLocation, 1, GL_FALSE, projectionViewMatrix) );

                glViewport( 0, 0, (GLsizei)width, (GLsizei)height );
            }
            else
            {
                ae_camera_t * camera = (ae_camera_t *)mesh.camera_data;

                float projectionViewMatrix[16];
                __mul_m4_m4( projectionViewMatrix, camera->projection, camera->view );

                GLCALL( glUniformMatrix4fv, (mvpMatrixLocation, 1, GL_FALSE, projectionViewMatrix) );

                glViewport( 0, 0, (GLsizei)camera->width, (GLsizei)camera->height );
            }

            GLuint opengl_indices_buffer_id = 0;
            GLCALL( glGenBuffers, (1, &opengl_indices_buffer_id) );
            GLCALL( glBindBuffer, (GL_ELEMENT_ARRAY_BUFFER, opengl_indices_buffer_id) );
            GLCALL( glBufferData, (GL_ELEMENT_ARRAY_BUFFER, opengl_indices_buffer_size, indices, GL_STREAM_DRAW) );

            GLCALL( glDrawElements, (GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_SHORT, 0) );

            GLCALL( glDisableVertexAttribArray, (positionLocation) );
            GLCALL( glDisableVertexAttribArray, (colorLocation) );
            GLCALL( glDisableVertexAttribArray, (texcoordLocation) );

            GLCALL( glActiveTexture, (GL_TEXTURE0) );
            GLCALL( glBindTexture, (GL_TEXTURE_2D, 0) );

            GLCALL( glUseProgram, (0) );

            GLCALL( glBindBuffer, (GL_ARRAY_BUFFER, 0) );
            GLCALL( glBindBuffer, (GL_ELEMENT_ARRAY_BUFFER, 0) );

            GLCALL( glDeleteBuffers, (1, &opengl_indices_buffer_id) );
            GLCALL( glDeleteBuffers, (1, &opengl_vertex_buffer_id) );
        }
        else
        {
            size_t opengl_indices_buffer_size = mesh.indexCount * sizeof( em_render_index_t );
            size_t opengl_vertex_buffer_size = mesh.vertexCount * sizeof( em_track_matte_render_vertex_t );

            em_node_track_matte_t * node_track_matte = (em_node_track_matte_t *)mesh.element_data;

            GLuint base_texture_id = node_track_matte->base_image->texture_id;
            GLuint track_matte_texture_id = node_track_matte->track_matte_image->texture_id;

            em_track_matte_render_vertex_t * vertices = player->track_matte_vertices;
            const ae_uint16_t * indices = mesh.indices;

            switch( mesh.layer_type )
            {
            case AE_MOVIE_LAYER_TYPE_SEQUENCE:
            case AE_MOVIE_LAYER_TYPE_IMAGE:
                {
                    const em_track_matte_t * track_matte = (const em_track_matte_t *)mesh.track_matte_data;

                    const aeMovieRenderMesh * track_matte_mesh = &track_matte->mesh;

                    uint32_t color = __make_argb( mesh.r, mesh.g, mesh.b, mesh.a * track_matte_mesh->a );

                    for( uint32_t index = 0; index != mesh.vertexCount; ++index )
                    {
                        em_track_matte_render_vertex_t * v = vertices + index;

                        const float * mesh_position = mesh.position[index];
                        __v3_v3_m4( v->position, mesh_position, player->wm );

                        v->color = color;

                        const float * mesh_uv = mesh.uv[index];

                        v->uv0[0] = mesh_uv[0];
                        v->uv0[1] = mesh_uv[1];

                        float uv1[2];
                        __calc_point_uv( uv1
                            , track_matte_mesh->position[0]
                            , track_matte_mesh->position[1]
                            , track_matte_mesh->position[2]
                            , track_matte_mesh->uv[0]
                            , track_matte_mesh->uv[1]
                            , track_matte_mesh->uv[2]
                            , mesh_position );

                        v->uv1[0] = uv1[0];
                        v->uv1[1] = uv1[1];
                    }
                }break;
            case AE_MOVIE_LAYER_TYPE_VIDEO:
                {
                }break;
            }

            GLuint opengl_vertex_buffer_id = 0;
            GLCALL( glGenBuffers, (1, &opengl_vertex_buffer_id) );
            GLCALL( glBindBuffer, (GL_ARRAY_BUFFER, opengl_vertex_buffer_id) );
            GLCALL( glBufferData, (GL_ARRAY_BUFFER, opengl_vertex_buffer_size, vertices, GL_STREAM_DRAW) );

            em_track_matte_shader_t * track_matte_shader = player->track_matte_shader;

            GLuint program_id = track_matte_shader->program_id;

            GLCALL( glUseProgram, (program_id) );

            GLint positionLocation = track_matte_shader->positionLocation;
            GLint colorLocation = track_matte_shader->colorLocation;
            GLint texcoord0Location = track_matte_shader->texcoord0Location;
            GLint texcoord1Location = track_matte_shader->texcoord1Location;
            GLint mvpMatrixLocation = track_matte_shader->mvpMatrixLocation;
            GLint tex0Location = track_matte_shader->tex0Location;
            GLint tex1Location = track_matte_shader->tex1Location;

            GLCALL( glActiveTexture, (GL_TEXTURE0) );
            GLCALL( glBindTexture, (GL_TEXTURE_2D, base_texture_id) );
            GLCALL( glUniform1i, (tex0Location, 0) );

            GLCALL( glActiveTexture, (GL_TEXTURE1) );
            GLCALL( glBindTexture, (GL_TEXTURE_2D, track_matte_texture_id) );
            GLCALL( glUniform1i, (tex1Location, 1) );

            GLCALL( glEnableVertexAttribArray, (positionLocation) );
            GLCALL( glEnableVertexAttribArray, (colorLocation) );
            GLCALL( glEnableVertexAttribArray, (texcoord0Location) );
            GLCALL( glEnableVertexAttribArray, (texcoord1Location) );

            GLCALL( glVertexAttribPointer, (positionLocation, 3, GL_FLOAT, GL_FALSE, sizeof( em_track_matte_render_vertex_t ), (const GLvoid *)offsetof( em_track_matte_render_vertex_t, position )) );
            GLCALL( glVertexAttribPointer, (colorLocation, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof( em_track_matte_render_vertex_t ), (const GLvoid *)offsetof( em_track_matte_render_vertex_t, color )) );
            GLCALL( glVertexAttribPointer, (texcoord0Location, 2, GL_FLOAT, GL_FALSE, sizeof( em_track_matte_render_vertex_t ), (const GLvoid *)offsetof( em_track_matte_render_vertex_t, uv0 )) );
            GLCALL( glVertexAttribPointer, (texcoord1Location, 2, GL_FLOAT, GL_FALSE, sizeof( em_track_matte_render_vertex_t ), (const GLvoid *)offsetof( em_track_matte_render_vertex_t, uv1 )) );

            if( mesh.camera_data == AE_NULL )
            {
                const aeMovieCompositionData * ae_movie_composition_data = ae_get_movie_composition_composition_data( ae_movie_composition );

                float viewMatrix[16];
                __identity_m4( viewMatrix );

                float composition_width = ae_get_movie_composition_data_width( ae_movie_composition_data );
                float composition_height = ae_get_movie_composition_data_height( ae_movie_composition_data );

                float width = player->width;
                float height = player->height;

                float projectionMatrix[16];
                __make_orthogonal_m4( projectionMatrix, width, height );

                float projectionViewMatrix[16];
                __mul_m4_m4( projectionViewMatrix, projectionMatrix, viewMatrix );

                GLCALL( glUniformMatrix4fv, (mvpMatrixLocation, 1, GL_FALSE, projectionViewMatrix) );

                glViewport( 0, 0, (GLsizei)width, (GLsizei)height );
            }
            else
            {
                ae_camera_t * camera = (ae_camera_t *)mesh.camera_data;

                float projectionViewMatrix[16];
                __mul_m4_m4( projectionViewMatrix, camera->projection, camera->view );

                GLCALL( glUniformMatrix4fv, (mvpMatrixLocation, 1, GL_FALSE, projectionViewMatrix) );

                glViewport( 0, 0, (GLsizei)camera->width, (GLsizei)camera->height );
            }

            GLuint opengl_indices_buffer_id = 0;
            GLCALL( glGenBuffers, (1, &opengl_indices_buffer_id) );
            GLCALL( glBindBuffer, (GL_ELEMENT_ARRAY_BUFFER, opengl_indices_buffer_id) );
            GLCALL( glBufferData, (GL_ELEMENT_ARRAY_BUFFER, opengl_indices_buffer_size, indices, GL_STREAM_DRAW) );

            GLCALL( glDrawElements, (GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_SHORT, 0) );

            GLCALL( glDisableVertexAttribArray, (positionLocation) );
            GLCALL( glDisableVertexAttribArray, (colorLocation) );
            GLCALL( glDisableVertexAttribArray, (texcoord0Location) );
            GLCALL( glDisableVertexAttribArray, (texcoord1Location) );

            GLCALL( glActiveTexture, (GL_TEXTURE0) );
            GLCALL( glBindTexture, (GL_TEXTURE_2D, 0) );
            GLCALL( glActiveTexture, (GL_TEXTURE1) );
            GLCALL( glBindTexture, (GL_TEXTURE_2D, 0) );

            GLCALL( glUseProgram, (0) );

            GLCALL( glBindBuffer, (GL_ARRAY_BUFFER, 0) );
            GLCALL( glBindBuffer, (GL_ELEMENT_ARRAY_BUFFER, 0) );

            GLCALL( glDeleteBuffers, (1, &opengl_indices_buffer_id) );
            GLCALL( glDeleteBuffers, (1, &opengl_vertex_buffer_id) );
        }
    }
}
//////////////////////////////////////////////////////////////////////////
void em_utils_opengl_create_texture( uint32_t _id, uint32_t _width, uint32_t _height, const uint8_t * _data )
{
    GLCALL( glBindTexture, (GL_TEXTURE_2D, _id) );
    GLCALL( glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE) );
    GLCALL( glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE) );
    GLCALL( glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR) );
    GLCALL( glTexParameteri, (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) );

    GLCALL( glTexImage2D, (GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0x00000000, GL_RGBA, GL_UNSIGNED_BYTE, _data) );
    
    GLCALL( glBindTexture, (GL_TEXTURE_2D, 0U) );
}