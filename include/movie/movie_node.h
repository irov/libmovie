#ifndef MOVIE_MOVIE_NODE_H_
#define MOVIE_MOVIE_NODE_H_

#	include <movie/movie_type.h>
#	include <movie/movie_instance.h>
#	include <movie/movie_resource.h>
#	include <movie/movie_data.h>

#	ifndef AE_MOVIE_MAX_VERTICES
#	define AE_MOVIE_MAX_VERTICES 64
#	endif

#	ifndef AE_MOVIE_MAX_RENDER_NODE
#	define AE_MOVIE_MAX_RENDER_NODE 1024
#	endif

#ifdef __cplusplus
extern "C" {
#endif

	typedef enum
	{
		AE_MOVIE_NODE_ANIMATE_STATIC,
		AE_MOVIE_NODE_ANIMATE_BEGIN,
		AE_MOVIE_NODE_ANIMATE_PROCESS,
		AE_MOVIE_NODE_ANIMATE_END,
		__AE_MOVIE_NODE_ANIMATE_STATES__
	} aeMovieNodeAnimationStateEnum;

	typedef struct aeMovieNodeCamera
	{
		void * data;
	} aeMovieNodeCamera;

	typedef struct aeMovieNode
	{
		const aeMovieLayerData * layer;

		struct aeMovieNode * relative;
		struct aeMovieNode * track_matte;
		struct aeMovieNode * prev_track_matte;

		float start_time;
		float in_time;
		float out_time;

		float stretch;
		float current_time;

		ae_bool_t active;

		uint32_t animate;

		uint32_t matrix_revision;
		ae_matrix4_t matrix;

		float composition_opactity;
		float opacity;

		aeMovieBlendMode blend_mode;

		const void * camera_data;
		void * element_data;
	} aeMovieNode;

	typedef void * (*ae_movie_composition_node_camera_provider_t)(const ae_string_t _name, const ae_vector3_t _position, const ae_vector3_t _direction, float _fov, float _width, float _height, void * _data);
	
	typedef void * (*ae_movie_composition_node_provider_t)(const aeMovieLayerData * _layerData, const aeMovieResource * _resource, const ae_matrix4_t _matrix, void * _data);
	
	typedef void( *ae_movie_node_animate_update_t )(const void * _element, uint32_t _type, const ae_matrix4_t _matrix, float _opacity, void * _data);
	typedef void( *ae_movie_node_animate_begin_t )(const void * _element, uint32_t _type, float _offset, void * _data);
	typedef void( *ae_movie_node_animate_end_t )(const void * _element, uint32_t _type, void * _data);

	typedef void( *ae_movie_composition_node_destroyer_t )(const void * _element, uint32_t _type, void * _data);

	typedef void( *ae_movie_node_event_t )(const void * _element, const char * _name, const ae_matrix4_t _matrix, float _opacity, ae_bool_t _begin, void * _data );

	typedef enum
	{
		AE_MOVIE_COMPOSITION_PLAY,
		AE_MOVIE_COMPOSITION_STOP,
		AE_MOVIE_COMPOSITION_PAUSE,
		AE_MOVIE_COMPOSITION_RESUME,
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

		ae_movie_node_animate_update_t animate_update;
		ae_movie_node_animate_begin_t animate_begin;
		ae_movie_node_animate_end_t animate_end;

		ae_movie_node_event_t event;

		ae_movie_composition_state_t composition_state;

	} aeMovieCompositionProviders;

	typedef struct aeMovieComposition
	{
		const aeMovieData * movie_data;
		const aeMovieCompositionData * composition_data;

		ae_bool_t play;
		ae_bool_t pause;
		ae_bool_t interrupt;

		ae_bool_t loop;

		uint32_t update_revision;
		float time;
		
		uint32_t node_count;
		aeMovieNode * nodes;

		aeMovieCompositionProviders providers;
		void * provider_data;
	} aeMovieComposition;

	aeMovieComposition * create_movie_composition( const aeMovieData * _movieData, const aeMovieCompositionData * _compositionData, const aeMovieCompositionProviders * providers, void * _data );
	void destroy_movie_composition( const aeMovieComposition * _composition );

	void set_movie_composition_loop( aeMovieComposition * _composition, ae_bool_t _loop );

	void play_movie_composition( aeMovieComposition * _composition, float _timing );
	void stop_movie_composition( aeMovieComposition * _composition );
	void pause_movie_composition( aeMovieComposition * _composition );
	void resume_movie_composition( aeMovieComposition * _composition );
	void interrupt_movie_composition( aeMovieComposition * _composition );

	void set_movie_composition_timing( aeMovieComposition * _composition, float _timing );

	void update_movie_composition( aeMovieComposition * _composition, float _timing );

	ae_bool_t set_movie_composition_slot( aeMovieComposition * _composition, const char * _slotName, void * _slotData );
	void * get_movie_composition_slot( aeMovieComposition * _composition, const char * _slotName );
	ae_bool_t has_movie_composition_slot( aeMovieComposition * _composition, const char * _slotName );
	void * remove_movie_composition_slot( aeMovieComposition * _composition, const char * _slotName );
	
	typedef struct aeMovieRenderContext
	{
		const aeMovieComposition * composition;
		
		float sprite_uv[8];
		uint16_t sprite_indices[6];

		uint32_t render_count;
		uint32_t render_node_indices[AE_MOVIE_MAX_RENDER_NODE];		
	} aeMovieRenderContext;

	aeMovieResult begin_movie_render_context( const aeMovieComposition * _composition, aeMovieRenderContext * _context );

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
		void * element_data;
	} aeMovieRenderMesh;

	void compute_movie_mesh( const aeMovieRenderContext * _context, uint32_t _index, aeMovieRenderMesh * _vertices );
	
#ifdef __cplusplus
}
#endif

#endif