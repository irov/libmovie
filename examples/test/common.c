/*
==================================================
	EXAMPLE COMMONS

User-written callback functions for libmovie and other stuff used in all the examples.
==================================================
*/

#include "common.h"
#include <malloc.h>
#include <memory.h>
#include <time.h>

//
// Declare working structure of our application.
//

examples_t ex;

//
// Playform-dependent function implementations.
//

//
// System time in milliseconds.
//
int ex_get_time (void) {
	return (int)((double)clock() / CLOCKS_PER_SEC);
}

static void * stdlib_movie_alloc( void * _data, size_t _size ) {
	(void)_data;
	return malloc( _size );
}

static void * stdlib_movie_alloc_n( void * _data, size_t _size, size_t _count ) {
	(void)_data;
	uint32_t total = _size * _count;
	return malloc( total );
}

static void stdlib_movie_free( void * _data, const void * _ptr ) {
	(void)_data;
	free( (void *)_ptr );
}

static void stdlib_movie_free_n( void * _data, const void * _ptr ) {
	(void)_data;
	free( (void *)_ptr );
}

static void stdlib_movie_logerror( void * _data, aeMovieErrorCode _code, const char * _format, ... ) {
	(void)_data;
	(void)_code;
	va_list argList;
	va_start( argList, _format );
	vprintf( _format, argList );
	va_end( argList );
}

static size_t read_file( void * _data, void * _buff, uint32_t _size ) {
	FILE * f = (FILE *)_data;
	size_t s = fread( _buff, 1, _size, f );
	return s;
}

static void memory_copy( void * _data, const void * _src, void * _dst, size_t _size ) {
	(void)_data;
	memcpy( _dst, _src, _size );
}

//==================================================

//
// Empty callback.
//
void * ex_callback_resource_provider_empty( const aeMovieResource * _resource, void * _data ) {
	(void)_resource;
	(void)_data;

	EX_LOG("Resource provider callback.\n");

	return AE_NULL;
}

//
// In this one, show information about resources being loaded.
//
// Return value goes into layer->resource_data, which in turn is used in node provider & aeMovieRenderMesh structure.
//
void * ex_callback_resource_provider( const aeMovieResource * _resource, void * _data ) {
	//
	// This would be our examples_t object if we used it here.
	//

	(void)_data;

	EX_LOG("Resource provider callback.\n");

	switch(_resource->type) {
		case AE_MOVIE_RESOURCE_IMAGE:
		{
			const aeMovieResourceImage * r = (const aeMovieResourceImage *)_resource;
			
			EX_LOG("Resource type: image.\n");
			EX_LOG(" path        = '%s'\n", r->path);
			EX_LOG(" trim_width  = %i\n", (int)r->trim_width);
			EX_LOG(" trim_height = %i\n", (int)r->trim_height);
			
			break;
		}
		case AE_MOVIE_RESOURCE_SEQUENCE:
		{
			EX_LOG("Resource type: image sequence.\n");

			break;
		}
		case AE_MOVIE_RESOURCE_VIDEO:
		{
//			const aeMovieResourceVideo * r = (const aeMovieResourceVideo *)_resource;

			EX_LOG("Resource type: video.\n");

			break;
		}
		case AE_MOVIE_RESOURCE_SOUND:
		{
			const aeMovieResourceSound * r = (const aeMovieResourceSound *)_resource;

			EX_LOG("Resource type: sound.\n");
			EX_LOG(" path        = '%s'", r->path);

			break;
		}
		case AE_MOVIE_RESOURCE_SLOT:
		{
			const aeMovieResourceSlot * r = (const aeMovieResourceSlot *)_resource;

			EX_LOG("Resource type: slot.\n");
			EX_LOG(" width  = %.2f\n", r->width);
			EX_LOG(" height = %.2f\n", r->height);

			break;
		}
		default:
		{
			EX_LOG("Resource type: other (%i).\n", _resource->type); 
			break;
		}
	}

	return AE_NULL;
}

//==================================================

void * ex_callback_camera_provider( const aeMovieCameraProviderCallbackData * _callbackData, void * _data ) {
	//
	// This would be our examples_t object if we used it here.
	//

	(void)_data;

	EX_LOG("Camera provider callback.\n");

	EX_LOG(" Camera:    '%s'\n", _callbackData->name);
	EX_LOG(" Position:  %.2f %.2f %.2f\n", _callbackData->position[0], _callbackData->position[1], _callbackData->position[2]);
	EX_LOG(" Direction: %.2f %.2f %.2f\n", _callbackData->direction[0], _callbackData->direction[1], _callbackData->direction[2]);
	EX_LOG(" Fov:       %.2f\n", _callbackData->fov);
	EX_LOG(" Width:     %.2f\n", _callbackData->width);
	EX_LOG(" Height:    %.2f\n", _callbackData->height);

	return NULL;
}

void * ex_callback_node_provider( const aeMovieNodeProviderCallbackData * _callbackData, void * _data ) {
	(void)_data;

	EX_LOG("Node provider callback.\n");


	if( ae_is_movie_layer_data_track_mate(_callbackData->layer) == AE_TRUE ) {
		EX_LOG(" Is track matte layer.\n");
		return NULL;
	}

	aeMovieLayerTypeEnum layerType = ae_get_movie_layer_data_type(_callbackData->layer);

	EX_LOG(" Layer: '%s'", ae_get_movie_layer_data_name(_callbackData->layer));

	if( _callbackData->trackmatteLayer == NULL ) {
		EX_LOG(" Has track matte: no\n");

		EX_LOG(" Type:");

		switch( layerType ) {
			case AE_MOVIE_LAYER_TYPE_SLOT:
			{
				EX_LOG(" slot\n");
				break;
			}
			case AE_MOVIE_LAYER_TYPE_VIDEO: {
				EX_LOG(" video\n");
				break;
			}
			case AE_MOVIE_LAYER_TYPE_SOUND: {
//				ex_sound_t *sound = (ex_sound_t *)ae_get_movie_layer_data_resource_data(_callbackData->layer);
				
				EX_LOG(" sound\n");
//				EX_LOG("  name: '%s'", sound->path);

				break;
			}
			default:
				EX_LOG(" other\n");
				break;
		}
	}
	else {
		EX_LOG(" Has track matte: yes\n");

		EX_LOG(" Type:");

		switch( layerType ) {
			case AE_MOVIE_LAYER_TYPE_SHAPE:
				EX_LOG(" shape\n");
				break;
			case AE_MOVIE_LAYER_TYPE_IMAGE:
				//
				// Image pointer was set in resource provider previously.
				//

//				ex_image_t * image = (ex_image_t *)ae_get_movie_layer_data_resource_data(_callbackData->trackmatteLayer);

				EX_LOG(" image\n");
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
				EX_LOG(" other\n");
				break;
		}
	}

	return NULL;
}

void ex_callback_node_destroyer( const aeMovieNodeDestroyCallbackData * _callbackData, void * _data ) {
	(void)_data;

	EX_LOG("Node destroyer callback.\n");
	EX_LOG(" Layer type: %i\n", _callbackData->type);
}

void ex_callback_node_update( const aeMovieNodeUpdateCallbackData * _callbackData, void * _data ) {
	(void)_data;

	EX_LOG("Node update callback.\n");
	EX_LOG(" State:");

	switch (_callbackData->state)
	{
		case AE_MOVIE_NODE_UPDATE_UPDATE:
		{
			EX_LOG(" NODE_UPDATE_UPDATE\n");
			EX_LOG(" Type:");

			switch (_callbackData->type) {
				case AE_MOVIE_LAYER_TYPE_PARTICLE:
					EX_LOG(" particle\n");
					break;
				case AE_MOVIE_LAYER_TYPE_SLOT:
					EX_LOG(" slot\n");
					break;
				default:
					EX_LOG(" other\n");
					break;
			}

			break;
		}
		case AE_MOVIE_NODE_UPDATE_BEGIN:
		{
			EX_LOG(" NODE_UPDATE_BEGIN\n");
			EX_LOG(" Type:");

			switch (_callbackData->type)
			{
				case AE_MOVIE_LAYER_TYPE_VIDEO:
					EX_LOG(" video\n");
					break;
				case AE_MOVIE_LAYER_TYPE_SOUND:
//					ex_sound_node_t *sound_node = (ex_sound_node_t *)_callbackData->element;

					EX_LOG(" sound\n");
//					EX_LOG("  name: '%s'\n", sound_node->sound->path);
					EX_LOG("  offset: %.2f seconds\n", _callbackData->offset * 0.001f);

					break;
				default:
					EX_LOG(" other\n");
					break;
			}

			break;
		}
		case AE_MOVIE_NODE_UPDATE_END:
		{
			EX_LOG(" NODE_UPDATE_END\n");
			EX_LOG(" Type:");

			switch (_callbackData->type)
			{
				case AE_MOVIE_LAYER_TYPE_VIDEO:
					EX_LOG(" video\n");
					break;
				case AE_MOVIE_LAYER_TYPE_SOUND:
//					ex_sound_node_t *sound_node = (ex_sound_node_t *)_callbackData->element;

					EX_LOG(" sound\n");
//					EX_LOG("  name: '%s'\n", sound_node->sound->path);

					break;
				default:
					EX_LOG(" other\n");
					break;
			}

			break;
		}
	}
}

void * ex_callback_track_matte_update( const aeMovieTrackMatteUpdateCallbackData * _callbackData, void * _data ) {
	(void)_data;

	EX_LOG("Track matte update callback.\n");

	EX_LOG(" State:");

	switch (_callbackData->state) {
		case AE_MOVIE_NODE_UPDATE_CREATE: {
			EX_LOG(" NODE_UPDATE_CREATE\n");

			//
			// Create a reference here to a user-provided structure.
			// It should contain the mesh data from the callback & be used during updating & rendering.
			// Return value goes into layer->track_matte_data.
			//

// 			ex_track_matte_data_t track_matte_data = ;
//			track_matte_data->mesh = *_callbackData->mesh;
//			return track_matte_data;

			break;
		}
		case AE_MOVIE_NODE_UPDATE_BEGIN: {
			EX_LOG(" NODE_UPDATE_BEGIN\n");
			
			// Should update track_matte_data->mesh with callback one.

			break;
		}
		case AE_MOVIE_NODE_UPDATE_UPDATE: {
			EX_LOG(" NODE_UPDATE_UPDATE\n");

			// Should update track_matte_data->mesh with callback one.

			break;
		}
		case AE_MOVIE_NODE_UPDATE_END: {
			EX_LOG(" NODE_UPDATE_END\n");

			// Do nothing.

			break;
		}
	}

	return NULL;
}

void ex_callback_composition_event( const aeMovieCompositionEventCallbackData * _callbackData, void * _data ) {
	(void)_data;

	EX_LOG("Composition event callback.\n");

	EX_LOG("Event: '%s'\n", _callbackData->name);
	EX_LOG(" opacity: %.2f\n", _callbackData->opacity);
	EX_LOG(" begin: %s\n", _callbackData->begin ? "TRUE" : "FALSE");
}

void ex_callback_composition_state( const aeMovieCompositionStateCallbackData * _callbackData, void * _data ) {
	(void)_data;

	EX_LOG("Composition state callback.\n");

	EX_LOG(" State:");

	//
	// If it references a subcomposition, then we should only work with that.
	//

	if (_callbackData->subcomposition != NULL) {
		switch (_callbackData->state) {
			case AE_MOVIE_SUB_COMPOSITION_PLAY:
				EX_LOG(" SUB_COMPOSITION_PLAY\n");
				break;
			case AE_MOVIE_SUB_COMPOSITION_STOP:
				EX_LOG(" SUB_COMPOSITION_STOP\n");
				break;
			case AE_MOVIE_SUB_COMPOSITION_PAUSE:
				EX_LOG(" SUB_COMPOSITION_PAUSE\n");
				break;
			case AE_MOVIE_SUB_COMPOSITION_RESUME:
				EX_LOG(" SUB_COMPOSITION_RESUME\n");
				break;
			case AE_MOVIE_SUB_COMPOSITION_INTERRUPT:
				EX_LOG(" SUB_COMPOSITION_INTERRUPT\n");
				break;
			case AE_MOVIE_SUB_COMPOSITION_END:
				EX_LOG(" SUB_COMPOSITION_END\n");
				break;
			case AE_MOVIE_SUB_COMPOSITION_LOOP_END:
				EX_LOG(" SUB_COMPOSITION_LOOP_END\n");
				break;
		}

		return;
	}

	//
	// Work with master composition here.
	//

	switch (_callbackData->state) {
		case AE_MOVIE_COMPOSITION_PLAY:
			EX_LOG(" COMPOSITION_PLAY\n");
			break;
		case AE_MOVIE_COMPOSITION_STOP:
			EX_LOG(" COMPOSITION_STOP\n");
			break;
		case AE_MOVIE_COMPOSITION_PAUSE:
			EX_LOG(" COMPOSITION_PAUSE\n");
			break;
		case AE_MOVIE_COMPOSITION_RESUME:
			EX_LOG(" COMPOSITION_RESUME\n");
			break;
		case AE_MOVIE_COMPOSITION_INTERRUPT:
			EX_LOG(" COMPOSITION_INTERRUPT\n");
			break;
		case AE_MOVIE_COMPOSITION_END:
			EX_LOG(" COMPOSITION_END\n");
			break;
		case AE_MOVIE_COMPOSITION_LOOP_END:
			EX_LOG(" COMPOSITION_LOOP_END\n");
			break;
	}
}

//==================================================

void ex_create_instance (void) {
	EX_LOG("Creating library instance.\n");

	ex.instance = ae_create_movie_instance( EX_LICENSE_KEY, &stdlib_movie_alloc, &stdlib_movie_alloc_n, &stdlib_movie_free, &stdlib_movie_free_n, (ae_movie_strncmp_t)AE_NULL, &stdlib_movie_logerror, AE_NULL );
}

//
// This loads .AEM into a data structure which your resource manager should have a list of, similar to image or sound cache.
//
void ex_load_movie_data (void) {
	EX_LOG("Opening file '%s'.\n", ex.movie_path);

	FILE * f = fopen(ex.movie_path, "rb" );

	if( f == NULL ) {
		EX_LOG("...failed.\n");
		exit(0);
	}

	EX_LOG("Creating movie stream.\n");

	aeMovieStream * stream = ae_create_movie_stream( ex.instance, &read_file, &memory_copy, f );

	EX_LOG("Loading movie data.\n");

	aeMovieData * data = ae_create_movie_data( ex.instance );

	if( ae_load_movie_data( data, stream, ex.resource_provider, NULL ) == AE_MOVIE_FAILED ) {
		EX_LOG("...failed.\n");

		ae_delete_movie_data( data );
		ae_delete_movie_stream( stream );
		fclose( f );

		exit(0);
	}

//	EX_LOG("Deleting movie stream.\n");

	ae_delete_movie_stream( stream );

//	EX_LOG("Closing file '%s'.\n", path);

	fclose( f );

	ex.data = data;
}

void ex_set_composition(void) {
	EX_LOG("Getting data for composition '%s'.\n", ex.comp_name);

	const aeMovieCompositionData * comp_data = ae_get_movie_composition_data( ex.data, ex.comp_name );

	if( comp_data == AE_NULL ) {
		EX_LOG("...failed.\n");
		exit(0);
	}

	EX_LOG("Creating composition.\n");

	aeMovieComposition * composition = ae_create_movie_composition( ex.data, comp_data, AE_TRUE, &ex.comp_providers, AE_NULL );

	if (composition == AE_NULL) {
		EX_LOG("...failed.\n");
		exit(0);
	}

	//
	// Show composition information.
	//

	EX_LOG("Composition '%s' information.\n", ex.comp_name);
	EX_LOG(" Duration: %.2f seconds\n", ae_get_movie_composition_duration(composition) * 0.001f);

	ex.composition = composition;
}

//
// The update loop.
//
void ex_update (int dt) {
	EX_LOG("\n====== Beginning of frame update =================\n\n");

	ae_update_movie_composition( ex.composition, (float)dt);

	EX_LOG("\n====== End of frame update =======================\n\n");
}

//
// The render loop.
//
void ex_render (void) {
	uint32_t render_mesh_it = 0;
	aeMovieRenderMesh render_mesh;

	EX_LOG("\n====== Beginning of frame render =================\n\n");

	while( ae_compute_movie_mesh( ex.composition, &render_mesh_it, &render_mesh ) == AE_TRUE ) {
		EX_LOG("Rendering mesh %i.\n", render_mesh_it);

		if( render_mesh.camera_data != NULL )
			EX_LOG(" Camera: yes\n");
		else
			EX_LOG(" Camera: no\n");

		EX_LOG(" Track matte: ");

		if( render_mesh.track_matte_data == NULL ) {
			//
			// layer has no track matte
			//

			EX_LOG(" no\n");

			EX_LOG(" Layer type: ");

			switch( render_mesh.layer_type )
			{
				case AE_MOVIE_LAYER_TYPE_ANY:
				{
					EX_LOG("any\n");
					break;
				}
				case AE_MOVIE_LAYER_TYPE_MOVIE:
				{
					EX_LOG("movie\n");
					break;
				}
				case AE_MOVIE_LAYER_TYPE_SHAPE:
				{
					EX_LOG("shape\n");
					break;
				}
				case AE_MOVIE_LAYER_TYPE_SLOT:
				{
					EX_LOG("slot\n");

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
					EX_LOG("solid\n");
					break;
				}
				case AE_MOVIE_LAYER_TYPE_SEQUENCE:
				case AE_MOVIE_LAYER_TYPE_IMAGE:
				{
					if (render_mesh.layer_type == AE_MOVIE_LAYER_TYPE_SEQUENCE)
						EX_LOG("image sequence\n");
					else
						EX_LOG("image\n");

					if (render_mesh.vertexCount == 0 || render_mesh.indexCount == 0)
						break;

					EX_LOG("Mesh info:\n");
					EX_LOG(" Vertex count = %i\n", render_mesh.vertexCount);
					EX_LOG(" Index count = %i\n", render_mesh.indexCount);
					EX_LOG(" Color: %.2f %.2f %.2f %.2f\n", render_mesh.r, render_mesh.g, render_mesh.b, render_mesh.a);
					EX_LOG(" Blend func: %i\n", render_mesh.blend_mode);

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
					EX_LOG("video\n");
					break;
				}
				case AE_MOVIE_LAYER_TYPE_SOUND:
				{
					EX_LOG("sound\n");
					break;
				}
				case AE_MOVIE_LAYER_TYPE_PARTICLE:
				{
					EX_LOG("particle\n");
					break;
				}
				case AE_MOVIE_LAYER_TYPE_SUB_MOVIE:
				{
					EX_LOG("submovie\n");
					break;
				}
			}
		}
		else {
			//
			// layer has track matte
			//

			EX_LOG(" yes\n");

			EX_LOG(" Layer type: ");

			switch( render_mesh.layer_type )
			{
				case AE_MOVIE_LAYER_TYPE_IMAGE:
				{
					EX_LOG("image\n");

					//
					// Reference to track matte descriptor (which usually contains just a texture pointer)
					// is in render_mesh.element_data.
					//

					void * track_matte_ref = render_mesh.element_data;

					if (track_matte_ref == NULL)
						break;
					if (render_mesh.vertexCount == 0 || render_mesh.indexCount == 0)
						break;

					EX_LOG("Mesh info:\n");
					EX_LOG(" Vertex count = %i\n", render_mesh.vertexCount);
					EX_LOG(" Index count = %i\n", render_mesh.indexCount);
					EX_LOG(" Color: %.2f %.2f %.2f %.2f\n", render_mesh.r, render_mesh.g, render_mesh.b, render_mesh.a);
					EX_LOG(" Blend func: %i\n", render_mesh.blend_mode);

					//
					// Mesh image pointer is in render_mesh.resource_data.
					// It comes from the source layer and was set in
					// resource provider callback previously.
					//

					//
					// Track matte data (which contains aeMovieRenderMesh of the masking layer)
					// is in render_mesh.track_matte_data.
					//

					void * track_matte_data = render_mesh.track_matte_data;
//					const aeMovieRenderMesh * track_matte_render_mesh = &track_matte_data->render_mesh;

					//
					// Track matte image pointer is in track_matte_render_mesh.resource_data.
					// It was set in node provider callback previously.
					//

//					if (track_matte_render_mesh.vertexCount == 0 || track_matte_render_mesh.indexCount == 0)
//						break;

					//
					// From here on you can use track matte mesh & texture to mask the main mesh rendering
					// in any way you want (multitexturing with reprojected UVs, multipass etc.).
					//

					// Render here using the main mesh & track matte data.

					break;
				}
			}
		}

		EX_LOG( "\n");
	}

	EX_LOG("\n====== End of frame render =======================\n\n");
}

//
// Called at the application start.
//
void ex_init (void) {
	atexit(ex_shutdown);

	//
	// Initialize the library instance.
	//

	ex_create_instance();

	//
	// Set source file & composition name.
	//

//	snprintf(ex.movie_path, sizeof(ex.movie_path), "AEM/ui/ui.aem");
//	snprintf(ex.comp_name, sizeof(ex.comp_name), "BigWin");
	snprintf(ex.movie_path, sizeof(ex.movie_path), "AEM/06_KitchenDoorZoom (converted)/06_KitchenDoorZoom (converted).aem");
	snprintf(ex.comp_name, sizeof(ex.comp_name), "Lomik_Use1");
}

//
// Called at the application exit.
//
void ex_shutdown (void) {
	//
	// Wait for a keypress.
	//
	
	EX_LOG(	"\nPress ENTER to exit...\n");

	getchar();

	//
	// Free the allocated memory.
	//

	if (ex.composition) {
		EX_LOG("Deleting composition.\n");
		ae_destroy_movie_composition( ex.composition );
	}

	if (ex.data) {
		EX_LOG("Deleting movie data.\n");
		ae_delete_movie_data( ex.data );
	}

	if (ex.instance) {
		EX_LOG("Deleting library instance.\n");
		ae_delete_movie_instance( ex.instance );
	}
}
