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

	aeMovieComposition * create_movie_composition( const aeMovieInstance * _instance, const aeMovieData * _data, const aeMovieCompositionData * _composition );
	void destroy_movie_composition( const aeMovieInstance * _instance, const aeMovieComposition * _composition );

	void set_movie_composition_loop( aeMovieComposition * _composition, ae_bool_t _loop );

	void update_movie_composition( aeMovieComposition * _composition, float _timing );
	
	typedef struct aeMovieRenderContext
	{
		const aeMovieComposition * composition;
		
		float sprite_uv[8];
		uint16_t sprite_indices[6];

		uint32_t render_node_indices[AE_MOVIE_MAX_RENDER_NODE];

		uint32_t mesh_count;
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
	} aeMovieRenderMesh;

	void compute_movie_mesh( const aeMovieRenderContext * _context, uint32_t _index, aeMovieRenderMesh * _vertices );
	
#ifdef __cplusplus
}
#endif

#endif