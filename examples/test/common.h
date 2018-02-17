#include "movie/movie.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

static const ae_char_t * ex_example_license_hash = "52ad6f051099762d0a0787b4eb2d07c8a0ee4491";
static const ae_char_t * ex_example_movie_path = "../../examples/resources/Knight/Knight.aem";
static const ae_char_t * ex_example_composition_name = "Knight";


ae_void_t EX_LOG( const char * _format, ... );

//
// Working structure of our application.
//

typedef struct {
	//
	// Movie parameters.
	//

	ae_char_t license[41];
	ae_char_t license_padding[3];
	ae_char_t movie_path[256];
	ae_char_t comp_name[128];

	//
	// Current working objects
	//

    const aeMovieInstance * instance;
	aeMovieData * data;
	aeMovieComposition * composition;

	//
	// Callbacks for data loading & setup.
	//

	ae_movie_data_resource_provider_t resource_provider;
    ae_movie_data_resource_deleter_t resource_deleter;
	aeMovieCompositionProviders comp_providers;
} examples_t;

extern examples_t ex;

//
// Platform-dependent functions.
//

ae_float_t ex_get_time( ae_void_t );

//
// Callbacks for loading movie resource data (images, videos, sounds etc.)
// This data might be shared among different compositions.
//

ae_voidptr_t ex_callback_resource_provider_empty( const aeMovieResource * _resource, ae_voidptr_t _data );
ae_void_t ex_callback_resource_deleter_empty( aeMovieResourceTypeEnum _type, ae_voidptr_t _data, ae_voidptr_t _ud );
ae_voidptr_t ex_callback_resource_provider( const aeMovieResource * _resource, ae_voidptr_t _data );

//
// Callbacks for loading composition data & managing it.
//

ae_voidptr_t ex_callback_camera_provider( const aeMovieCameraProviderCallbackData * _callbackData, ae_voidptr_t _data );
ae_voidptr_t ex_callback_node_provider( const aeMovieNodeProviderCallbackData * _callbackData, ae_voidptr_t _data );
ae_void_t ex_callback_node_destroyer( const aeMovieNodeDeleterCallbackData * _callbackData, ae_voidptr_t _data );
ae_void_t ex_callback_node_update( const aeMovieNodeUpdateCallbackData * _callbackData, ae_voidptr_t _data );
ae_voidptr_t ex_callback_track_matte_provider( const aeMovieTrackMatteProviderCallbackData * _callbackData, ae_voidptr_t _data );
ae_void_t ex_callback_track_matte_deleter( const aeMovieTrackMatteDeleterCallbackData * _callbackData, ae_voidptr_t _data );
ae_void_t ex_callback_track_matte_update( const aeMovieTrackMatteUpdateCallbackData * _callbackData, ae_voidptr_t _data );
ae_void_t ex_callback_composition_event( const aeMovieCompositionEventCallbackData * _callbackData, ae_voidptr_t _data );
ae_void_t ex_callback_composition_state( const aeMovieCompositionStateCallbackData * _callbackData, ae_voidptr_t _data );

//
// Miscellaneous & shared functions.
//

ae_void_t ex_create_instance( ae_void_t );
ae_void_t ex_load_movie_data( ae_void_t );
ae_void_t ex_set_composition( ae_void_t );
ae_void_t ex_update( ae_float_t dt );
ae_void_t ex_render( ae_void_t );

ae_void_t ex_init( const ae_char_t * license, const ae_char_t * path, const ae_char_t * composition );
ae_void_t ex_shutdown( ae_void_t );
