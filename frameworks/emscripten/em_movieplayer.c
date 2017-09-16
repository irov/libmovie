#   include "em_movieplayer.h"

#   include "movie/movie.h"

#   include <malloc.h>
#   include <string.h>

#   include <GLES2/gl2.h>

#   include <SDL/SDL_image.h>

//////////////////////////////////////////////////////////////////////////
static void * __instance_alloc( void * _data, size_t _size )
{
    (void)_data;

    return malloc( _size );
}
//////////////////////////////////////////////////////////////////////////
static void * __instance_alloc_n( void * _data, size_t _size, size_t _count )
{
    (void)_data;

    size_t total = _size * _count;

    return malloc( total );
}
//////////////////////////////////////////////////////////////////////////
static void __instance_free( void * _data, const void * _ptr )
{
    (void)_data;

    free( (void *)_ptr );
}
//////////////////////////////////////////////////////////////////////////
static void __instance_free_n( void * _data, const void * _ptr )
{
    (void)_data;

    free( (void *)_ptr );
}
//////////////////////////////////////////////////////////////////////////
static void __instance_logerror( void * _data, aeMovieErrorCode _code, const char * _format, ... )
{
    (void)_data;
    (void)_code;
    (void)_format;
}
//////////////////////////////////////////////////////////////////////////
em_movie_instance_t em_create_movie_instance( const char * _hashkey )
{
    aeMovieInstance * ae_instance = ae_create_movie_instance( _hashkey
        , &__instance_alloc
        , &__instance_alloc_n
        , &__instance_free
        , &__instance_free_n
        , AE_NULL
        , &__instance_logerror
        , AE_NULL );

    //emscripten_log( EM_LOG_CONSOLE, "test" );
    emscripten_log( EM_LOG_CONSOLE, "successful create movie instance" );

    IMG_Init( IMG_INIT_PNG );

    return ae_instance;
}
//////////////////////////////////////////////////////////////////////////
void em_delete_movie_instance( em_movie_instance_t _instance )
{
    aeMovieInstance * ae_instance = (aeMovieInstance *)_instance;
    
    ae_delete_movie_instance( ae_instance );

    emscripten_log( EM_LOG_CONSOLE, "successful delete movie instance" );
}
//////////////////////////////////////////////////////////////////////////
static ae_size_t __read_file( ae_voidptr_t _data, ae_voidptr_t _buff, ae_uint32_t _carriage, ae_uint32_t _size ) 
{
    (void)_carriage;

    FILE * f = (FILE *)_data;

    ae_size_t s = fread( _buff, 1, _size, f );

    //emscripten_log( EM_LOG_CONSOLE, "read_file %u", _size );

    return s;
}
//////////////////////////////////////////////////////////////////////////
static void __memory_copy( ae_voidptr_t _data, ae_constvoidptr_t _src, ae_voidptr_t _dst, ae_size_t _size ) 
{
    (void)_data;
    memcpy( _dst, _src, _size );

    //emscripten_log( EM_LOG_CONSOLE, "memory_copy %u", _size );
}
//////////////////////////////////////////////////////////////////////////
typedef struct em_resource_image_data
{
    GLuint texture_id;
} em_resource_image_data;
//////////////////////////////////////////////////////////////////////////
static ae_voidptr_t __data_resource_provider( const aeMovieResource * _resource, ae_voidptr_t _ud ) {
    (void)_resource;
    (void)_ud;

    switch( _resource->type ) 
    {
    case AE_MOVIE_RESOURCE_IMAGE:
        {
            const aeMovieResourceImage * r = (const aeMovieResourceImage *)_resource;

            emscripten_log( EM_LOG_CONSOLE, "Resource type: image.\n" );
            emscripten_log( EM_LOG_CONSOLE, " path        = '%s'\n", r->path );
            emscripten_log( EM_LOG_CONSOLE, " trim_width  = %f\n", r->trim_width );
            emscripten_log( EM_LOG_CONSOLE, " trim_height = %f\n", r->trim_height );

            em_resource_image_data * em_resource = ( em_resource_image_data * )malloc( sizeof( em_resource_image_data ) );

            SDL_Surface * surface = IMG_Load( r->path );

            if( surface == NULL )
            {
                emscripten_log( EM_LOG_CONSOLE, "bad load image '%s'\n", r->path );
            }

            emscripten_log( EM_LOG_CONSOLE, "create texture %ux%u (%u)\n", surface->w, surface->h, surface->format->BytesPerPixel );

            GLuint texture_id;
            glGenTextures( 1, &texture_id );
            glBindTexture( GL_TEXTURE_2D, texture_id );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

            glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels );

            SDL_FreeSurface( surface );
            
            em_resource->texture_id = texture_id;

            return em_resource;

            break;
        }
    case AE_MOVIE_RESOURCE_SEQUENCE:
        {
            emscripten_log( EM_LOG_CONSOLE, "Resource type: image sequence.\n" );

            break;
        }
    case AE_MOVIE_RESOURCE_VIDEO:
        {
            //			const aeMovieResourceVideo * r = (const aeMovieResourceVideo *)_resource;

            emscripten_log( EM_LOG_CONSOLE, "Resource type: video.\n" );

            break;
        }
    case AE_MOVIE_RESOURCE_SOUND:
        {
            const aeMovieResourceSound * r = (const aeMovieResourceSound *)_resource;

            emscripten_log( EM_LOG_CONSOLE, "Resource type: sound.\n" );
            emscripten_log( EM_LOG_CONSOLE, " path        = '%s'", r->path );

            break;
        }
    case AE_MOVIE_RESOURCE_SLOT:
        {
            const aeMovieResourceSlot * r = (const aeMovieResourceSlot *)_resource;

            emscripten_log( EM_LOG_CONSOLE, "Resource type: slot.\n" );
            emscripten_log( EM_LOG_CONSOLE, " width  = %.2f\n", r->width );
            emscripten_log( EM_LOG_CONSOLE, " height = %.2f\n", r->height );

            break;
        }
    default:
        {
            emscripten_log( EM_LOG_CONSOLE, "Resource type: other (%i).\n", _resource->type );
            break;
        }
    }

    return AE_NULL;
}
//////////////////////////////////////////////////////////////////////////
static void __data_resource_deleter( aeMovieResourceTypeEnum _type, const ae_voidptr_t * _data, ae_voidptr_t _ud )
{    
    (void)_data;
    (void)_ud;

    switch( _type )
    {
    case AE_MOVIE_RESOURCE_IMAGE:
        {
            em_resource_image_data * image_data = (em_resource_image_data *)_data;

            glDeleteTextures( 1, &image_data->texture_id );

            free( (ae_voidptr_t *)_data );

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
em_movie_data_t em_create_movie_data( em_movie_instance_t _instance, const char * _path )
{
    aeMovieInstance * ae_instance = (aeMovieInstance *)_instance;

    aeMovieData * ae_movie_data = ae_create_movie_data( ae_instance, &__data_resource_provider, &__data_resource_deleter, AE_NULL );
    
    FILE * f = fopen( _path, "rb" );

    if( f == NULL )
    {
        emscripten_log( EM_LOG_CONSOLE, "invalid open file (%s)", _path );

        return AE_NULL;
    }

    fseek( f, 0L, SEEK_END );
    size_t sz = ftell( f );
    rewind( f );

    ae_voidptr_t * buffer = malloc( sz );
    fread( buffer, sz, 1, f );
    fclose( f );

    aeMovieStream * ae_stream = ae_create_movie_stream_memory( ae_instance, buffer, sz, &__memory_copy, AE_NULL );

    if( ae_load_movie_data( ae_movie_data, ae_stream ) == AE_MOVIE_FAILED )
    {
        emscripten_log( EM_LOG_CONSOLE, "WRONG LOAD (%s)", _path );
    }

    ae_delete_movie_stream( ae_stream );

    free( buffer );

    emscripten_log( EM_LOG_CONSOLE, "successful create movie data from file '%s'", _path );

    return ae_movie_data;
}
//////////////////////////////////////////////////////////////////////////
void em_delete_movie_data( em_movie_data_t _movieData )
{
    aeMovieData * ae_movie_data = (aeMovieData *)_movieData;

    ae_delete_movie_data( ae_movie_data );

    emscripten_log( EM_LOG_CONSOLE, "successful delete movie data" );
}
//////////////////////////////////////////////////////////////////////////
em_movie_composition_t em_create_movie_composition( em_movie_data_t _movieData, const char * _name )
{
    const aeMovieCompositionData * movieCompositionData = ae_get_movie_composition_data( _movieData, _name );

    aeMovieCompositionProviders compositionProviders;
    ae_initialize_movie_composition_providers( &compositionProviders );

    aeMovieComposition * movieComposition = ae_create_movie_composition( _movieData, movieCompositionData, AE_TRUE, &compositionProviders, AE_NULL );

    return movieComposition;
}
//////////////////////////////////////////////////////////////////////////
EMSCRIPTEN_KEEPALIVE void em_delete_movie_composition( em_movie_composition_t _movieComposition )
{
    ae_delete_movie_composition( _movieComposition );
}
//////////////////////////////////////////////////////////////////////////
EMSCRIPTEN_KEEPALIVE void em_set_movie_composition_loop( em_movie_composition_t _movieComposition, ae_bool_t _loop )
{
    ae_set_movie_composition_loop( _movieComposition, _loop );
}