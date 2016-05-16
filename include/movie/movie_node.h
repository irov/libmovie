#ifndef MOVIE_MOVIE_NODE_H_
#define MOVIE_MOVIE_NODE_H_

#	include <movie/movie_type.h>
#	include <movie/movie_instance.h>
#	include <movie/movie_resource.h>
#	include <movie/movie_data.h>

#	ifndef AE_MOVIE_MAX_VERTICES
#	define AE_MOVIE_MAX_VERTICES 64
#	endif

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct aeMovieRenderMesh
	{
		uint8_t layer_type;

		uint32_t animate;

		uint8_t blend_mode;

		uint8_t resource_type;
		void * resource_data;

		uint32_t vertexCount;
		uint32_t indexCount;

		float position[AE_MOVIE_MAX_VERTICES * 3];
		const float * uv;

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

	typedef void * (*ae_movie_composition_node_camera_provider_t)(const ae_string_t _name, const ae_vector3_t _position, const ae_vector3_t _direction, float _fov, float _width, float _height, void * _data);
	
	typedef void * (*ae_movie_composition_node_provider_t)(const aeMovieLayerData * _layerData, const aeMovieResource * _resource, const ae_matrix4_t _matrix, void * _data);
	

	typedef enum
	{
		AE_MOVIE_NODE_UPDATE_BEGIN = 0,
		AE_MOVIE_NODE_UPDATE_UPDATE = 1,
		AE_MOVIE_NODE_UPDATE_END = 2,
		__AE_MOVIE_NODE_UPDATE_STATES__,
	}aeMovieNodeUpdateState;

	typedef void( *ae_movie_composition_node_update_t )(const void * _element, uint32_t _type, aeMovieNodeUpdateState _state, float _offset, const ae_matrix4_t _matrix, float _opacity, void * _data);
	typedef void * ( *ae_movie_composition_track_matte_update_t )(const void * _element, uint32_t _type, aeMovieNodeUpdateState _state, float _offset, const aeMovieRenderMesh * _mesh, void * _track_matte_data, void * _data);

	typedef void( *ae_movie_composition_node_destroyer_t )(const void * _element, uint32_t _type, void * _data);

	typedef void( *ae_movie_node_event_t )(const void * _element, const char * _name, const ae_matrix4_t _matrix, float _opacity, ae_bool_t _begin, void * _data );

	typedef enum
	{
		AE_MOVIE_COMPOSITION_PLAY,
		AE_MOVIE_COMPOSITION_STOP,
		AE_MOVIE_COMPOSITION_INTERRUPT,
		AE_MOVIE_COMPOSITION_END,
		AE_MOVIE_COMPOSITION_LOOP_END,
		__AE_MOVIE_COMPOSITION_STATES__
	} aeMovieCompositionStateFlag;
	
	typedef void( *ae_movie_composition_state_t )( aeMovieCompositionStateFlag _state, void * _data);

	typedef struct aeMovieCompositionProviders
	{
		ae_movie_composition_node_camera_provider_t camera_provider;

		ae_movie_composition_node_provider_t node_provider;
		ae_movie_composition_node_destroyer_t node_destroyer;
		ae_movie_composition_node_update_t node_update;
		ae_movie_composition_track_matte_update_t track_matte_update;

		ae_movie_node_event_t event;

		ae_movie_composition_state_t composition_state;

	} aeMovieCompositionProviders;

	typedef struct aeMovieComposition aeMovieComposition;

	aeMovieComposition * ae_create_movie_composition( const aeMovieData * _movieData, const aeMovieCompositionData * _compositionData, const aeMovieCompositionProviders * providers, void * _data );
	void ae_destroy_movie_composition( const aeMovieComposition * _composition );

	ae_bool_t ae_get_movie_composition_anchor_point( const aeMovieComposition * _composition, ae_vector3_t _point );
	float ae_get_movie_composition_time( const aeMovieComposition * _composition );
	uint32_t ae_get_movie_composition_max_render_node( const aeMovieComposition * _composition );

	void ae_set_movie_composition_loop( aeMovieComposition * _composition, ae_bool_t _loop );

	void ae_play_movie_composition( aeMovieComposition * _composition, float _timing );
	void ae_stop_movie_composition( aeMovieComposition * _composition );
	void ae_interrupt_movie_composition( aeMovieComposition * _composition, ae_bool_t _skip );

	void ae_set_movie_composition_time( aeMovieComposition * _composition, float _timing );

	void ae_update_movie_composition( aeMovieComposition * _composition, float _timing );

	ae_bool_t ae_set_movie_composition_slot( aeMovieComposition * _composition, const char * _slotName, void * _slotData );
	void * ae_get_movie_composition_slot( aeMovieComposition * _composition, const char * _slotName );
	ae_bool_t ae_has_movie_composition_slot( aeMovieComposition * _composition, const char * _slotName );
	void * ae_remove_movie_composition_slot( aeMovieComposition * _composition, const char * _slotName );
	
	typedef struct aeMovieRenderContext aeMovieRenderContext;

	ae_bool_t ae_compute_movie_mesh( const aeMovieComposition * _composition, uint32_t * _iterator, aeMovieRenderMesh * _vertices );
	
#ifdef __cplusplus
}
#endif

#endif