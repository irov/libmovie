#ifndef MOVIE_MOVIE_NODE_H_
#define MOVIE_MOVIE_NODE_H_

#	include <movie/movie_type.h>
#	include <movie/movie_instance.h>
#	include <movie/movie_resource.h>
#	include <movie/movie_data.h>

#	define AE_MOVIE_MAX_VERTICES 64
#	define AE_MOVIE_MAX_RENDER_NODE 256

#ifdef __cplusplus
extern "C" {
#endif

	typedef enum aeMovieNodeAnimationStateEnum
	{
		AE_MOVIE_NODE_ANIMATE_STATIC,
		AE_MOVIE_NODE_ANIMATE_BEGIN,
		AE_MOVIE_NODE_ANIMATE_PROCESS,
		AE_MOVIE_NODE_ANIMATE_END
	};

	typedef struct aeMovieNodeCamera
	{
		void * data;
	} aeMovieNodeCamera;

	typedef struct aeMovieNode
	{
		const aeMovieLayerData * layer;

		struct aeMovieNode * relative;

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

		const void * camera_data;
		void * element_data;
	} aeMovieNode;

	typedef struct aeMovieComposition
	{
		const aeMovieData * movie_data;
		const aeMovieCompositionData * composition_data;

		ae_bool_t loop;

		uint32_t update_revision;
		float timing;
		
		uint32_t node_count;
		aeMovieNode * nodes;
	} aeMovieComposition;

	aeMovieComposition * create_movie_composition( const aeMovieInstance * _instance, const aeMovieData * _movieData, const aeMovieCompositionData * _compositionData );
	void destroy_movie_composition( const aeMovieInstance * _instance, const aeMovieComposition * _composition );

	typedef void * (*ae_movie_composition_node_camera_t)(const ae_string_t _name, const ae_vector3_t _position, const ae_vector3_t _direction, float _fov, float _width, float _height, void * _data);
	typedef void * (*ae_movie_composition_node_video_t)(const aeMovieLayerData * _layerData, const aeMovieResourceVideo * _resource, void * _data);

	typedef struct aeMovieCompositionNodeProvider
	{
		ae_movie_composition_node_camera_t camera_provider;
		ae_movie_composition_node_video_t video_provider;

	} aeMovieCompositionNodeProvider;

	void create_movie_composition_element( aeMovieComposition * _composition, const aeMovieCompositionNodeProvider * _provider, void * _data );

	void set_movie_composition_loop( aeMovieComposition * _composition, ae_bool_t _loop );

	typedef void( *begin_movie_node_animate_t )(const void * _element, uint32_t _type, void * _data);
	typedef void( *end_movie_node_animate_t )(const void * _element, uint32_t _type, void * _data);

	void update_movie_composition( aeMovieComposition * _composition, float _timing, begin_movie_node_animate_t _begin, end_movie_node_animate_t _end, void * _data );
	
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