#ifndef MOVIE_MOVIE_NODE_H_
#define MOVIE_MOVIE_NODE_H_

#	include <movie/movie_type.h>
#	include <movie/movie_instance.h>
#	include <movie/movie_resource.h>
#	include <movie/movie_data.h>

#	define AE_MOVIE_MAX_VERTICES 64
#	define AE_MOVIE_MAX_INDICES 96

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct aeMovieNode
	{
		const aeMovieLayerData * layer;

		struct aeMovieNode * relative;

		float in_time;
		float out_time;

		uint32_t matrix_revision;

		ae_matrix4_t matrix;		
		float opacity;
	} aeMovieNode;

	typedef struct aeMovieComposition
	{
		const aeMovieData * data;
		const aeMovieCompositionData * composition_data;

		uint32_t update_revision;
		float timing;

		uint32_t node_count;
		aeMovieNode * nodes;
	} aeMovieComposition;

	aeMovieComposition * create_movie_composition( const aeMovieInstance * _instance, const aeMovieData * _data, const aeMovieCompositionData * _composition );
	void delete_movie_composition( const aeMovieInstance * _instance, const aeMovieComposition * _composition );

	void update_movie_composition( aeMovieComposition * _composition, float _timing );
	
	typedef struct aeMovieRenderContext
	{
		const aeMovieComposition * composition;

		uint32_t render_node_iterator;
	} aeMovieRenderContext;

	void begin_movie_render_context( const aeMovieComposition * _composition, aeMovieRenderContext * _context );

	typedef struct aeMovieRenderNode
	{
		uint8_t layer_type;

		uint8_t resource_type;
		void * resource_data;
	} aeMovieRenderNode;

	ae_bool_t next_movie_redner_context( aeMovieRenderContext * _context, aeMovieRenderNode * _renderNode );
	
	typedef struct aeMovieRenderVertices
	{
		uint32_t vertexCount;
		uint32_t indexCount;

		float position[AE_MOVIE_MAX_VERTICES * 3];
		float uv[AE_MOVIE_MAX_VERTICES * 2];

		uint16_t indices[AE_MOVIE_MAX_INDICES];

		float r;
		float g;
		float b;
		float a;
	} aeMovieRenderVertices;

	void compute_movie_vertices( const aeMovieRenderContext * _context, aeMovieRenderVertices * _vertices );
	
#ifdef __cplusplus
}
#endif

#endif