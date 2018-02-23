/*
==================================================
    EXAMPLE COMMONS

User-written callback functions for libmovie and other stuff used in all the examples.
==================================================
*/

#include "common.h"
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4820)
#endif

#include <time.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

//
// Declare working structure of our application.
//

examples_t ex;

//
// Playform-dependent function implementations.
//

//#define TEST_WITHOUT_LOG

ae_void_t EX_LOG( const char * _format, ... )
{
#ifdef TEST_WITHOUT_LOG
    ( ae_void_t )_format;
#else
    va_list argList;
    va_start( argList, _format );
    vprintf( _format, argList );
    va_end( argList );
#endif
}

//
// System time in milliseconds.
//
ae_float_t ex_get_time( ae_void_t ) {
    clock_t c = clock();
    return (float)c / (float)CLOCKS_PER_SEC;
}

AE_CALLBACK ae_voidptr_t stdlib_movie_alloc( ae_voidptr_t _data, ae_size_t _size ) {
    AE_UNUSED( _data );
    return malloc( _size );
}

AE_CALLBACK ae_voidptr_t stdlib_movie_alloc_n( ae_voidptr_t _data, ae_size_t _size, ae_size_t _count ) {
    AE_UNUSED( _data );
    ae_uint32_t total = _size * _count;
    return malloc( total );
}

AE_CALLBACK ae_void_t stdlib_movie_free( ae_voidptr_t _data, ae_constvoidptr_t _ptr ) {
    AE_UNUSED( _data );
    free( (ae_voidptr_t)_ptr );
}

AE_CALLBACK ae_void_t stdlib_movie_free_n( ae_voidptr_t _data, ae_constvoidptr_t _ptr ) {
    AE_UNUSED( _data );
    free( (ae_voidptr_t)_ptr );
}

AE_CALLBACK ae_void_t stdlib_movie_logerror( ae_voidptr_t _data, aeMovieErrorCode _code, const ae_char_t * _format, ... ) {
    AE_UNUSED( _data );
    AE_UNUSED( _code );
    va_list argList;
    va_start( argList, _format );
    vprintf( _format, argList );
    va_end( argList );
}

AE_CALLBACK ae_size_t __read_file( ae_voidptr_t _data, ae_voidptr_t _buff, ae_size_t _carriage, ae_size_t _size ) {
    AE_UNUSED( _carriage );
    FILE * f = (FILE *)_data;
    ae_size_t s = fread( _buff, 1, _size, f );
    return s;
}

AE_CALLBACK ae_void_t __memory_copy( ae_voidptr_t _data, ae_constvoidptr_t _src, ae_voidptr_t _dst, ae_size_t _size ) {
    AE_UNUSED( _data );
    memcpy( _dst, _src, _size );
}

//==================================================

//
// Empty callback.
//
ae_voidptr_t ex_callback_resource_provider_empty( const aeMovieResource * _resource, ae_voidptr_t _data ) {
    AE_UNUSED( _resource );
    AE_UNUSED( _data );

    EX_LOG( "Resource provider callback.\n" );

    return AE_NULL;
}

ae_void_t ex_callback_resource_deleter_empty( aeMovieResourceTypeEnum _type, ae_voidptr_t _data, ae_voidptr_t _ud ) {
    AE_UNUSED( _type );
    AE_UNUSED( _data );
    AE_UNUSED( _ud );

    EX_LOG( "Resource deleter callback.\n" );
}

//
// In this one, show information about resources being loaded.
//
// Return value goes into layer->resource_data, which in turn is used in node provider & aeMovieRenderMesh structure.
//
ae_voidptr_t ex_callback_resource_provider( const aeMovieResource * _resource, ae_voidptr_t _data ) {
    //
    // This would be our examples_t object if we used it here.
    //

    AE_UNUSED( _data );

    EX_LOG( "Resource provider callback.\n" );

    switch( _resource->type ) {
    case AE_MOVIE_RESOURCE_IMAGE:
        {
            const aeMovieResourceImage * r = (const aeMovieResourceImage *)_resource;

            EX_LOG( "Resource type: image.\n" );
            EX_LOG( " path        = '%s'\n", r->path );
            EX_LOG( " trim_width  = %i\n", (int)r->trim_width );
            EX_LOG( " trim_height = %i\n", (int)r->trim_height );

            break;
        }
    case AE_MOVIE_RESOURCE_SEQUENCE:
        {
            EX_LOG( "Resource type: image sequence.\n" );

            break;
        }
    case AE_MOVIE_RESOURCE_VIDEO:
        {
            //			const aeMovieResourceVideo * r = (const aeMovieResourceVideo *)_resource;

            EX_LOG( "Resource type: video.\n" );

            break;
        }
    case AE_MOVIE_RESOURCE_SOUND:
        {
            const aeMovieResourceSound * r = (const aeMovieResourceSound *)_resource;

            EX_LOG( "Resource type: sound.\n" );
            EX_LOG( " path        = '%s'", r->path );

            break;
        }
    case AE_MOVIE_RESOURCE_SLOT:
        {
            const aeMovieResourceSlot * r = (const aeMovieResourceSlot *)_resource;

            EX_LOG( "Resource type: slot.\n" );
            EX_LOG( " width  = %.2f\n", r->width );
            EX_LOG( " height = %.2f\n", r->height );

            break;
        }
    default:
        {
            EX_LOG( "Resource type: other (%i).\n", _resource->type );
            break;
        }
    }

    return AE_NULL;
}

//==================================================

ae_voidptr_t ex_callback_camera_provider( const aeMovieCameraProviderCallbackData * _callbackData, ae_voidptr_t _data ) {
    //
    // This would be our examples_t object if we used it here.
    //

    AE_UNUSED( _data );

    EX_LOG( "Camera provider callback.\n" );

    EX_LOG( " Camera:    '%s'\n", _callbackData->name );
    EX_LOG( " Position:  %.2f %.2f %.2f\n", _callbackData->position[0], _callbackData->position[1], _callbackData->position[2] );
    EX_LOG( " Target: %.2f %.2f %.2f\n", _callbackData->target[0], _callbackData->target[1], _callbackData->target[2] );
    EX_LOG( " Quaternion: %.2f %.2f %.2f %.2f\n", _callbackData->quaternion[0], _callbackData->quaternion[1], _callbackData->quaternion[2], _callbackData->quaternion[3] );
    EX_LOG( " Fov:       %.2f\n", _callbackData->fov );
    EX_LOG( " Width:     %.2f\n", _callbackData->width );
    EX_LOG( " Height:    %.2f\n", _callbackData->height );

    return AE_NULL;
}

ae_voidptr_t ex_callback_node_provider( const aeMovieNodeProviderCallbackData * _callbackData, ae_voidptr_t _data ) {
    AE_UNUSED( _data );

    EX_LOG( "Node provider callback.\n" );


    if( ae_is_movie_layer_data_track_mate( _callbackData->layer ) == AE_TRUE ) {
        EX_LOG( " Is track matte layer.\n" );
        return AE_NULL;
    }

    aeMovieLayerTypeEnum layerType = ae_get_movie_layer_data_type( _callbackData->layer );

    EX_LOG( " Layer: '%s'", ae_get_movie_layer_data_name( _callbackData->layer ) );

    if( _callbackData->track_matte_layer == AE_NULL ) {
        EX_LOG( " Has track matte: no\n" );

        EX_LOG( " Type:" );

        switch( layerType ) {
        case AE_MOVIE_LAYER_TYPE_SLOT:
            {
                EX_LOG( " slot\n" );
                break;
            }
        case AE_MOVIE_LAYER_TYPE_VIDEO: {
                EX_LOG( " video\n" );
                break;
            }
        case AE_MOVIE_LAYER_TYPE_SOUND: {
                //				ex_sound_t *sound = (ex_sound_t *)ae_get_movie_layer_data_resource_data(_callbackData->layer);

                EX_LOG( " sound\n" );
                //				EX_LOG("  name: '%s'", sound->path);

                break;
            }
        default:
            EX_LOG( " other\n" );
            break;
        }
    }
    else {
        EX_LOG( " Has track matte: yes\n" );

        EX_LOG( " Type:" );

        switch( layerType ) {
        case AE_MOVIE_LAYER_TYPE_SHAPE:
            EX_LOG( " shape\n" );
            break;
        case AE_MOVIE_LAYER_TYPE_IMAGE:
            //
            // Image pointer was set in resource provider previously.
            //

//				ex_image_t * image = (ex_image_t *)ae_get_movie_layer_data_resource_data(_callbackData->trackmatteLayer);

            EX_LOG( " image\n" );
            //				EX_LOG("  name: '%s'\n", image->path);

                            //
                            // Create a reference here to a user-provided structure.
                            // It should contain the track matte image & be used during rendering, but not updating.
                            //

            //				ex_track_matte_ref_t track_matte_ref = ;
            //				track_matte_ref->image = image;
            //				return track_matte_ref;

            break;
        default:
            EX_LOG( " other\n" );
            break;
        }
    }

    return AE_NULL;
}

ae_void_t ex_callback_node_destroyer( const aeMovieNodeDeleterCallbackData * _callbackData, ae_voidptr_t _data ) {
    AE_UNUSED( _data );

    EX_LOG( "Node destroyer callback.\n" );

    aeMovieLayerTypeEnum type = ae_get_movie_layer_data_type( _callbackData->layer );

    EX_LOG( " Layer type: %i\n", type );
}

ae_void_t ex_callback_node_update( const aeMovieNodeUpdateCallbackData * _callbackData, ae_voidptr_t _data ) {
    AE_UNUSED( _data );

    EX_LOG( "Node update callback.\n" );
    EX_LOG( " State:" );

    switch( _callbackData->state )
    {
    case AE_MOVIE_STATE_UPDATE_BEGIN:
        {
            EX_LOG( " NODE_UPDATE_BEGIN\n" );
            EX_LOG( " Type:" );

            switch( _callbackData->type )
            {
            case AE_MOVIE_LAYER_TYPE_VIDEO:
                EX_LOG( " video\n" );
                break;
            case AE_MOVIE_LAYER_TYPE_SOUND:
                //                    ex_sound_node_t *sound_node = (ex_sound_node_t *)_callbackData->element;

                EX_LOG( " sound\n" );
                //                    EX_LOG("  name: '%s'\n", sound_node->sound->path);
                EX_LOG( "  offset: %.2f seconds\n", _callbackData->offset * 0.001f );

                break;
            default:
                EX_LOG( " other\n" );
                break;
            }
        }break;
    case AE_MOVIE_STATE_UPDATE_PROCESS:
        {
            EX_LOG( " NODE_UPDATE_UPDATE\n" );
            EX_LOG( " Type:" );

            switch( _callbackData->type ) {
            case AE_MOVIE_LAYER_TYPE_PARTICLE:
                EX_LOG( " particle\n" );
                break;
            case AE_MOVIE_LAYER_TYPE_SLOT:
                EX_LOG( " slot\n" );
                break;
            default:
                EX_LOG( " other\n" );
                break;
            }
        }break;
    case AE_MOVIE_STATE_UPDATE_PAUSE:
        {
        }break;
    case AE_MOVIE_STATE_UPDATE_RESUME:
        {
        }break;
    case AE_MOVIE_STATE_UPDATE_END:
        {
            EX_LOG( " NODE_UPDATE_END\n" );
            EX_LOG( " Type:" );

            switch( _callbackData->type )
            {
            case AE_MOVIE_LAYER_TYPE_VIDEO:
                EX_LOG( " video\n" );
                break;
            case AE_MOVIE_LAYER_TYPE_SOUND:
                //					ex_sound_node_t *sound_node = (ex_sound_node_t *)_callbackData->element;

                EX_LOG( " sound\n" );
                //					EX_LOG("  name: '%s'\n", sound_node->sound->path);

                break;
            default:
                EX_LOG( " other\n" );
                break;
            }
        }break;
    }
}

ae_voidptr_t ex_callback_track_matte_provider( const aeMovieTrackMatteProviderCallbackData * _callbackData, ae_voidptr_t _data ) {
    AE_UNUSED( _callbackData );
    AE_UNUSED( _data );

    EX_LOG( "Track matte provider callback.\n" );

    EX_LOG( " NODE_UPDATE_CREATE\n" );

    //
    // Create a reference here to a user-provided structure.
    // It should contain the mesh data from the callback & be used during updating & rendering.
    // Return value goes into layer->track_matte_data.
    //

    // 			ex_track_matte_data_t track_matte_data = ;
    //			track_matte_data->mesh = *_callbackData->mesh;
    //			return track_matte_data;

    return AE_NULL;
}


ae_void_t ex_callback_track_matte_update( const aeMovieTrackMatteUpdateCallbackData * _callbackData, ae_voidptr_t _data ) {
    AE_UNUSED( _data );

    EX_LOG( "Track matte update callback.\n" );

    EX_LOG( " State:" );

    switch( _callbackData->state )
    {
    case AE_MOVIE_STATE_UPDATE_BEGIN:
        {
            EX_LOG( " NODE_UPDATE_BEGIN\n" );

            // Should update track_matte_data->mesh with callback one.
        }break;
    case AE_MOVIE_STATE_UPDATE_PROCESS:
        {
            EX_LOG( " NODE_UPDATE_UPDATE\n" );

            // Should update track_matte_data->mesh with callback one.
        }break;
    case AE_MOVIE_STATE_UPDATE_PAUSE:
        {
            EX_LOG( " NODE_UPDATE_PAUSE\n" );

            // Should update track_matte_data->mesh with callback one.
        }break;
    case AE_MOVIE_STATE_UPDATE_RESUME:
        {
            EX_LOG( " NODE_UPDATE_RESUME\n" );

            // Should update track_matte_data->mesh with callback one.
        }break;
    case AE_MOVIE_STATE_UPDATE_END:
        {
            EX_LOG( " NODE_UPDATE_END\n" );

            // Do nothing.
        }break;
    }
}

ae_void_t ex_callback_track_matte_deleter( const aeMovieTrackMatteDeleterCallbackData * _callbackData, ae_voidptr_t _data ) {
    AE_UNUSED( _callbackData );
    AE_UNUSED( _data );

    EX_LOG( "Track matte deleter callback.\n" );
}


ae_void_t ex_callback_composition_event( const aeMovieCompositionEventCallbackData * _callbackData, ae_voidptr_t _data ) {
    AE_UNUSED( _data );

    EX_LOG( "Composition event callback.\n" );

    EX_LOG( "Event: '%s'\n", _callbackData->name );
    EX_LOG( " opacity: %.2f\n", _callbackData->opacity );
    EX_LOG( " begin: %s\n", _callbackData->begin ? "TRUE" : "FALSE" );
}

ae_void_t ex_callback_composition_state( const aeMovieCompositionStateCallbackData * _callbackData, ae_voidptr_t _data ) {
    AE_UNUSED( _data );

    EX_LOG( "Composition state callback.\n" );

    EX_LOG( " State:" );

    //
    // If it references a subcomposition, then we should only work with that.
    //

    if( _callbackData->subcomposition != AE_NULL ) {
        switch( _callbackData->state ) {
        case AE_MOVIE_COMPOSITION_PLAY:
            EX_LOG( " SUB_COMPOSITION_PLAY\n" );
            break;
        case AE_MOVIE_COMPOSITION_STOP:
            EX_LOG( " SUB_COMPOSITION_STOP\n" );
            break;
        case AE_MOVIE_COMPOSITION_PAUSE:
            EX_LOG( " SUB_COMPOSITION_PAUSE\n" );
            break;
        case AE_MOVIE_COMPOSITION_RESUME:
            EX_LOG( " SUB_COMPOSITION_RESUME\n" );
            break;
        case AE_MOVIE_COMPOSITION_INTERRUPT:
            EX_LOG( " SUB_COMPOSITION_INTERRUPT\n" );
            break;
        case AE_MOVIE_COMPOSITION_END:
            EX_LOG( " SUB_COMPOSITION_END\n" );
            break;
        case AE_MOVIE_COMPOSITION_LOOP_END:
            EX_LOG( " SUB_COMPOSITION_LOOP_END\n" );
            break;
        }

        return;
    }

    //
    // Work with master composition here.
    //

    switch( _callbackData->state ) {
    case AE_MOVIE_COMPOSITION_PLAY:
        EX_LOG( " COMPOSITION_PLAY\n" );
        break;
    case AE_MOVIE_COMPOSITION_STOP:
        EX_LOG( " COMPOSITION_STOP\n" );
        break;
    case AE_MOVIE_COMPOSITION_PAUSE:
        EX_LOG( " COMPOSITION_PAUSE\n" );
        break;
    case AE_MOVIE_COMPOSITION_RESUME:
        EX_LOG( " COMPOSITION_RESUME\n" );
        break;
    case AE_MOVIE_COMPOSITION_INTERRUPT:
        EX_LOG( " COMPOSITION_INTERRUPT\n" );
        break;
    case AE_MOVIE_COMPOSITION_END:
        EX_LOG( " COMPOSITION_END\n" );
        break;
    case AE_MOVIE_COMPOSITION_LOOP_END:
        EX_LOG( " COMPOSITION_LOOP_END\n" );
        break;
    }
}

//==================================================

ae_void_t ex_create_instance( ae_void_t ) {
    EX_LOG( "Creating library instance.\n" );

    ex.instance = ae_create_movie_instance( ex.license
        , &stdlib_movie_alloc
        , &stdlib_movie_alloc_n
        , &stdlib_movie_free
        , &stdlib_movie_free_n
        , (ae_movie_strncmp_t)AE_NULL
        , &stdlib_movie_logerror
        , AE_NULL );
}

//
// This loads .AEM into a data structure which your resource manager should have a list of, similar to image or sound cache.
//
ae_void_t ex_load_movie_data( ae_void_t ) {
    EX_LOG( "Opening file '%s'.\n", ex.movie_path );

    FILE * f = fopen( ex.movie_path, "rb" );

    if( f == NULL ) {
        EX_LOG( "...failed.\n" );
        exit( 0 );
    }

    EX_LOG( "Creating movie stream.\n" );

    aeMovieStream * stream = ae_create_movie_stream( ex.instance, &__read_file, &__memory_copy, f );

    EX_LOG( "Loading movie data.\n" );

    aeMovieData * data = ae_create_movie_data( ex.instance, ex.resource_provider, ex.resource_deleter, AE_NULL );

    ae_uint32_t load_version;
	ae_result_t load_movie_data_result = ae_load_movie_data( data, stream, &load_version );

    if( load_movie_data_result != AE_RESULT_SUCCESSFUL ) {
        const ae_char_t * load_movie_data_result_info = ae_get_result_string_info( load_movie_data_result );
        EX_LOG( "%s\n", load_movie_data_result_info );
        EX_LOG( "...failed.\n" );

        ae_delete_movie_data( data );
        ae_delete_movie_stream( stream );
        fclose( f );

        exit( 0 );
    }

    //	EX_LOG("Deleting movie stream.\n");

    ae_delete_movie_stream( stream );

    //	EX_LOG("Closing file '%s'.\n", path);

    fclose( f );

    ex.data = data;
}

ae_void_t ex_set_composition( ae_void_t ) {
    EX_LOG( "Getting data for composition '%s'.\n", ex.comp_name );

    const aeMovieCompositionData * comp_data = ae_get_movie_composition_data( ex.data, ex.comp_name );

    if( comp_data == AE_NULL ) {
        EX_LOG( "...failed.\n" );
        exit( 0 );
    }

    EX_LOG( "Creating composition.\n" );

    aeMovieComposition * composition = ae_create_movie_composition( ex.data, comp_data, AE_TRUE, &ex.comp_providers, AE_NULL );

    if( composition == AE_NULL ) {
        EX_LOG( "...failed.\n" );
        exit( 0 );
    }

    //
    // Show composition information.
    //

    EX_LOG( "Composition '%s' information.\n", ex.comp_name );
    EX_LOG( " Duration: %.2f seconds\n", ae_get_movie_composition_duration( composition ) * 0.001f );

    ex.composition = composition;
}

//
// The update loop.
//
ae_void_t ex_update( ae_float_t dt ) {
    EX_LOG( "\n====== Beginning of frame update =================\n\n" );
    EX_LOG( "dt: %f", dt );

    ae_update_movie_composition( ex.composition, dt );

    EX_LOG( "\n====== End of frame update =======================\n\n" );
}

//
// The render loop.
//
ae_void_t ex_render( ae_void_t ) {
    ae_uint32_t render_mesh_it = 0;
    aeMovieRenderMesh render_mesh;

    EX_LOG( "\n====== Beginning of frame render =================\n\n" );

    while( ae_compute_movie_mesh( ex.composition, &render_mesh_it, &render_mesh ) == AE_TRUE ) {
        EX_LOG( "Rendering mesh %i.\n", render_mesh_it );

        //if( render_mesh.camera_data != AE_NULL )
        //	EX_LOG( " Camera: yes\n" );
        //else
        //	EX_LOG( " Camera: no\n" );

        EX_LOG( " Track matte: " );

        if( render_mesh.track_matte_data == AE_NULL ) {
            //
            // layer has no track matte
            //

            EX_LOG( " no\n" );

            EX_LOG( " Layer type: " );

            switch( render_mesh.layer_type )
            {
            case AE_MOVIE_LAYER_TYPE_MOVIE:
                {
                    EX_LOG( "movie\n" );
                    break;
                }
            case AE_MOVIE_LAYER_TYPE_SHAPE:
                {
                    EX_LOG( "shape\n" );
                    break;
                }
            case AE_MOVIE_LAYER_TYPE_SLOT:
                {
                    EX_LOG( "slot\n" );

                    //					ex_slot_node_t * slot_node = (ex_slot_node_t *)render_mesh.element_data;

                                        //
                                        // Here you should visit/draw the slot node
                                        // (which is also a sub-class of your node class)
                                        // to render itself & its children, i.e. the nodes
                                        // that you attached to this slot.
                                        //

                    //					visit_node(slot_node);

                    break;
                }
            case AE_MOVIE_LAYER_TYPE_SOLID:
                {
                    EX_LOG( "solid\n" );
                    break;
                }
            case AE_MOVIE_LAYER_TYPE_SEQUENCE:
            case AE_MOVIE_LAYER_TYPE_IMAGE:
                {
                    if( render_mesh.layer_type == AE_MOVIE_LAYER_TYPE_SEQUENCE )
                        EX_LOG( "image sequence\n" );
                    else
                        EX_LOG( "image\n" );

                    if( render_mesh.vertexCount == 0 || render_mesh.indexCount == 0 )
                        break;

                    EX_LOG( "Mesh info:\n" );
                    EX_LOG( " Vertex count = %i\n", render_mesh.vertexCount );
                    EX_LOG( " Index count = %i\n", render_mesh.indexCount );
                    EX_LOG( " Color: %.2f %.2f %.2f %.2f\n", render_mesh.color.r, render_mesh.color.g, render_mesh.color.b, render_mesh.opacity );
                    EX_LOG( " Blend func: %i\n", render_mesh.blend_mode );

                    //
                    // Mesh image pointer is in render_mesh.resource_data.
                    // It comes from the source layer and was set in
                    // resource provider callback previously.
                    //

                    // Render here using the mesh data.

                    break;
                }
            case AE_MOVIE_LAYER_TYPE_VIDEO:
                {
                    EX_LOG( "video\n" );
                    break;
                }
            case AE_MOVIE_LAYER_TYPE_SOUND:
                {
                    EX_LOG( "sound\n" );
                    break;
                }
            case AE_MOVIE_LAYER_TYPE_PARTICLE:
                {
                    EX_LOG( "particle\n" );
                    break;
                }
            case AE_MOVIE_LAYER_TYPE_SUB_MOVIE:
                {
                    EX_LOG( "submovie\n" );
                    break;
                }
            case AE_MOVIE_LAYER_TYPE_SPRITE: {
                    //Empty
                    break;
                }
            case AE_MOVIE_LAYER_TYPE_TEXT: {
                    //Empty
                    break;
                }
            case AE_MOVIE_LAYER_TYPE_EVENT: {
                    //Empty
                    break;
                }
            case AE_MOVIE_LAYER_TYPE_SOCKET: {
                    //Empty
                    break;
                }
            case AE_MOVIE_LAYER_TYPE_NULL: {
                    //Empty
                    break;
                }
            }
        }
        else {
            //
            // layer has track matte
            //

            EX_LOG( " yes\n" );

            EX_LOG( " Layer type: " );

            switch( render_mesh.layer_type )
            {
            case AE_MOVIE_LAYER_TYPE_MOVIE:
            case AE_MOVIE_LAYER_TYPE_SPRITE:
            case AE_MOVIE_LAYER_TYPE_TEXT:
            case AE_MOVIE_LAYER_TYPE_EVENT:
            case AE_MOVIE_LAYER_TYPE_SOCKET:
            case AE_MOVIE_LAYER_TYPE_SHAPE:
            case AE_MOVIE_LAYER_TYPE_SLOT:
            case AE_MOVIE_LAYER_TYPE_NULL:
            case AE_MOVIE_LAYER_TYPE_SOLID:
            case AE_MOVIE_LAYER_TYPE_SEQUENCE:
            case AE_MOVIE_LAYER_TYPE_VIDEO:
            case AE_MOVIE_LAYER_TYPE_SOUND:
            case AE_MOVIE_LAYER_TYPE_PARTICLE: {
                    //Empty
                }break;
            case AE_MOVIE_LAYER_TYPE_IMAGE:
                {
                    EX_LOG( "image\n" );

                    //
                    // Reference to track matte descriptor (which usually contains just a texture pointer)
                    // is in render_mesh.element_data.
                    //

                    ae_voidptr_t track_matte_ref = render_mesh.element_data;

                    if( track_matte_ref == AE_NULL )
                        break;
                    if( render_mesh.vertexCount == 0 || render_mesh.indexCount == 0 )
                        break;

                    EX_LOG( "Mesh info:\n" );
                    EX_LOG( " Vertex count = %i\n", render_mesh.vertexCount );
                    EX_LOG( " Index count = %i\n", render_mesh.indexCount );
                    EX_LOG( " Color: %.2f %.2f %.2f %.2f\n", render_mesh.color.r, render_mesh.color.g, render_mesh.color.b, render_mesh.opacity );
                    EX_LOG( " Blend func: %i\n", render_mesh.blend_mode );

                    //
                    // Mesh image pointer is in render_mesh.resource_data.
                    // It comes from the source layer and was set in
                    // resource provider callback previously.
                    //

                    //
                    // Track matte data (which contains aeMovieRenderMesh of the masking layer)
                    // is in render_mesh.track_matte_data.
                    //

                    //ae_voidptr_t track_matte_data = render_mesh.track_matte_data;
                    //                    const aeMovieRenderMesh * track_matte_render_mesh = &track_matte_data->render_mesh;

                    //
                    // Track matte image pointer is in track_matte_render_mesh.resource_data.
                    // It was set in node provider callback previously.
                    //

                    //                    if (track_matte_render_mesh.vertexCount == 0 || track_matte_render_mesh.indexCount == 0)
                    //                        break;

                    //
                    // From here on you can use track matte mesh & texture to mask the main mesh rendering
                    // in any way you want (multitexturing with reprojected UVs, multipass etc.).
                    //

                    // Render here using the main mesh & track matte data.

                }break;
            case AE_MOVIE_LAYER_TYPE_SUB_MOVIE: {
                    //Empty                    
                }break;
            }
        }

        EX_LOG( "\n" );
    }

    EX_LOG( "\n====== End of frame render =======================\n\n" );
}

//
// Called at the application start.
//
ae_void_t ex_init( const ae_char_t * license, const ae_char_t * path, const ae_char_t * composition ) {
    //
    // Set source file & composition name.
    //

    strcpy( ex.license, license );
    strcpy( ex.movie_path, path );
    strcpy( ex.comp_name, composition );

    //
    // Initialize the library instance.
    //

    ex_create_instance();
}

//
// Called at the application exit.
//
ae_void_t ex_shutdown( ae_void_t ) {
    //
    // Wait for a keypress.
    //

    EX_LOG( "\nPress ENTER to exit...\n" );

	(void)getchar();

    //
    // Free the allocated memory.
    //

    if( ex.composition ) {
        EX_LOG( "Deleting movie composition.\n" );
        ae_delete_movie_composition( ex.composition );
        ex.composition = AE_NULL;
    }

	if( ex.data ) {
		EX_LOG( "Deleting movie data.\n" );
		ae_delete_movie_data( ex.data );
		ex.data = AE_NULL;
	}

    if( ex.data ) {
        EX_LOG( "Deleting movie data.\n" );
        ae_delete_movie_data( ex.data );
        ex.data = AE_NULL;
    }

    if( ex.instance ) {
        EX_LOG( "Deleting library instance.\n" );
        ae_delete_movie_instance( ex.instance );
        ex.instance = AE_NULL;
    }
}
