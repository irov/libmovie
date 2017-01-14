#ifndef MOVIE_MOVIE_NODE_H_
#define MOVIE_MOVIE_NODE_H_

#	include "movie_type.h"
#	include "movie_instance.h"
#	include "movie_resource.h"
#	include "movie_data.h"

typedef struct aeMovieRenderMesh
{
	uint8_t layer_type;

	aeMovieBlendMode blend_mode;

	uint8_t resource_type;
	void * resource_data;

	uint32_t vertexCount;
	uint32_t indexCount;

	ae_vector3_t position[AE_MOVIE_MAX_VERTICES];
	const ae_vector2_t * uv;

	const uint16_t * indices;

	float r;
	float g;
	float b;
	float a;

	const void * camera_data;
	const void * track_matte_data;

	void * element_data;
} aeMovieRenderMesh;

typedef struct aeMovieNode aeMovieNode;

typedef enum
{
	AE_MOVIE_NODE_UPDATE_BEGIN = 0,
	AE_MOVIE_NODE_UPDATE_UPDATE = 1,
	AE_MOVIE_NODE_UPDATE_END = 2,
	AE_MOVIE_NODE_UPDATE_CREATE = 3,
	AE_MOVIE_NODE_UPDATE_DESTROY = 4,
	__AE_MOVIE_NODE_UPDATE_STATES__,
}aeMovieNodeUpdateState;

typedef struct aeMovieCameraProviderCallbackData
{	
	const ae_char_t * name;
	ae_vector3_ptr_t position;
	ae_vector3_ptr_t direction;
	float fov;
	float width;
	float height;
} aeMovieCameraProviderCallbackData;

typedef struct aeMovieNodeProviderCallbackData
{
	const aeMovieLayerData * layer;
	ae_matrix4_ptr_t matrix; 
	float opacity;
	const aeMovieLayerData * trackmatteLayer;
} aeMovieNodeProviderCallbackData;

typedef struct aeMovieNodeDestroyCallbackData
{
	void * element;
	uint32_t type;
} aeMovieNodeDestroyCallbackData;

typedef struct aeMovieNodeUpdateCallbackData
{
	void * element;
	uint32_t type;
	ae_bool_t loop;
	aeMovieNodeUpdateState state;
	float offset;
	ae_matrix4_ptr_t matrix;
	float opacity;
} aeMovieNodeUpdateCallbackData;

typedef struct aeMovieTrackMatteUpdateCallbackData
{
	void * element;
	uint32_t type;
	ae_bool_t loop;
	aeMovieNodeUpdateState state;
	float offset;
	ae_matrix4_ptr_t matrix;
	aeMovieRenderMesh * mesh;
	void * track_matte_data;
} aeMovieTrackMatteUpdateCallbackData;

typedef struct aeMovieCompositionEventCallbackData
{
	void * element;
	const ae_char_t * name;
	ae_matrix4_ptr_t matrix;
	float opacity;
	ae_bool_t begin;
} aeMovieCompositionEventCallbackData;

typedef enum
{
	AE_MOVIE_COMPOSITION_PLAY,
	AE_MOVIE_COMPOSITION_STOP,
	AE_MOVIE_COMPOSITION_INTERRUPT,
	AE_MOVIE_COMPOSITION_END,
	AE_MOVIE_COMPOSITION_LOOP_END,
	AE_MOVIE_SUB_COMPOSITION_PLAY,
	AE_MOVIE_SUB_COMPOSITION_STOP,
	AE_MOVIE_SUB_COMPOSITION_INTERRUPT,
	AE_MOVIE_SUB_COMPOSITION_END,
	AE_MOVIE_SUB_COMPOSITION_LOOP_END,
	__AE_MOVIE_COMPOSITION_STATES__
} aeMovieCompositionStateFlag;

typedef struct aeMovieCompositionStateCallbackData
{
	aeMovieCompositionStateFlag state;
	const ae_char_t * subcomposition;
} aeMovieCompositionStateCallbackData;

typedef void * (*ae_movie_callback_camera_provider_t)(const aeMovieCameraProviderCallbackData * _callbackData, void * _data);
typedef void * (*ae_movie_callback_node_provider_t)(const aeMovieNodeProviderCallbackData * _callbackData, void * _data);
typedef void (*ae_movie_callback_node_destroy_t)(const aeMovieNodeDestroyCallbackData * _callbackData, void * _data);
typedef void (*ae_movie_callback_node_update_t)(const aeMovieNodeUpdateCallbackData * _callbackData, void * _data);
typedef void * (*ae_movie_callback_track_matte_update_t)(const aeMovieTrackMatteUpdateCallbackData * _callbackData, void * _data);
typedef void (*ae_movie_callback_composition_event_t)(const aeMovieCompositionEventCallbackData * _callbackData, void * _data);
typedef void (*ae_movie_callback_composition_state_t)(const aeMovieCompositionStateCallbackData * _callbackData, void * _data);

typedef struct aeMovieCompositionProviders
{
	ae_movie_callback_camera_provider_t camera_provider;

	ae_movie_callback_node_provider_t node_provider;
	ae_movie_callback_node_destroy_t node_destroyer;
	ae_movie_callback_node_update_t node_update;
	
	ae_movie_callback_track_matte_update_t track_matte_update;
		
	ae_movie_callback_composition_event_t composition_event;
	ae_movie_callback_composition_state_t composition_state;

} aeMovieCompositionProviders;

typedef struct aeMovieComposition aeMovieComposition;

aeMovieComposition * ae_create_movie_composition( const aeMovieData * _movieData, const aeMovieCompositionData * _compositionData, ae_bool_t _interpolate, const aeMovieCompositionProviders * providers, void * _data );
void ae_destroy_movie_composition( const aeMovieComposition * _composition );

ae_bool_t ae_get_movie_composition_anchor_point( const aeMovieComposition * _composition, ae_vector3_t _point );

uint32_t ae_get_movie_composition_max_render_node( const aeMovieComposition * _composition );

void ae_set_movie_composition_loop( aeMovieComposition * _composition, ae_bool_t _loop );
void ae_set_movie_composition_interpolate( aeMovieComposition * _composition, ae_bool_t _interpolate );

ae_bool_t ae_set_movie_composition_work_area( aeMovieComposition * _composition, float _begin, float _end );
void ae_remove_movie_composition_work_area( aeMovieComposition * _composition );

void ae_play_movie_composition( aeMovieComposition * _composition, float _time );
void ae_stop_movie_composition( aeMovieComposition * _composition );
void ae_interrupt_movie_composition(aeMovieComposition * _composition, ae_bool_t _skip, ae_bool_t _loop);

void ae_set_movie_composition_time( aeMovieComposition * _composition, float _timing );
float ae_get_movie_composition_time( const aeMovieComposition * _composition );

float ae_get_movie_composition_duration( const aeMovieComposition * _composition );

void ae_update_movie_composition( aeMovieComposition * _composition, float _timing );

ae_bool_t ae_set_movie_composition_slot( aeMovieComposition * _composition, const ae_char_t * _slotName, void * _slotData );
void * ae_get_movie_composition_slot( const aeMovieComposition * _composition, const ae_char_t * _slotName );
ae_bool_t ae_has_movie_composition_slot( const aeMovieComposition * _composition, const ae_char_t * _slotName );
void * ae_remove_movie_composition_slot( aeMovieComposition * _composition, const ae_char_t * _slotName );

ae_bool_t ae_get_movie_composition_socket( const aeMovieComposition * _composition, const ae_char_t * _slotName, const aeMoviePolygon ** _polygon );

ae_bool_t ae_compute_movie_mesh( const aeMovieComposition * _composition, uint32_t * _iterator, aeMovieRenderMesh * _vertices );
uint32_t ae_get_movie_render_mesh_count( const aeMovieComposition * _composition );

ae_bool_t ae_get_movie_composition_node_in_out_time( const aeMovieComposition * _composition, const ae_char_t * _layerName, aeMovieLayerTypeEnum _type, float * _in, float * _out );

void ae_play_movie_sub_composition( aeMovieComposition * _composition, const ae_char_t * _submovieName, float _time );
void ae_stop_movie_sub_composition( aeMovieComposition * _composition, const ae_char_t * _submovieName );
void ae_interrupt_movie_sub_composition( aeMovieComposition * _composition, const ae_char_t * _submovieName, ae_bool_t _skip, ae_bool_t _loop );

void ae_set_movie_sub_composition_time( aeMovieComposition * _composition, const ae_char_t * _submovieName, float _timing );
float ae_get_movie_sub_composition_time( const aeMovieComposition * _composition, const ae_char_t * _submovieName );

void ae_set_movie_sub_composition_loop( aeMovieComposition * _composition, const ae_char_t * _submovieName, ae_bool_t _loop );

ae_bool_t ae_set_movie_sub_composition_work_area( aeMovieComposition * _composition, const ae_char_t * _submovieName, float _begin, float _end );
void ae_remove_movie_sub_composition_work_area( aeMovieComposition * _composition, const ae_char_t * _submovieName );

#endif