#ifndef MOVIE_MOVIE_NODE_H_
#define MOVIE_MOVIE_NODE_H_

#	include <movie/movie_type.h>
#	include <movie/movie_instance.h>
#	include <movie/movie_resource.h>
#	include <movie/movie_data.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct aeMovieNode
	{
		const aeMovieLayerData * layer;

		struct aeMovieNode * relative;
	} aeMovieNode;

	typedef struct aeMovieComposition
	{
		const aeMovieData * data;
		const aeMovieCompositionData * composition_data;

		float timing;

		uint32_t node_count;
		aeMovieNode * nodes;
	} aeMovieComposition;

	aeMovieComposition * create_movie_composition( const aeMovieInstance * _instance, const aeMovieData * _data, const aeMovieCompositionData * _composition );
	void delete_movie_composition( const aeMovieInstance * _instance, const aeMovieNode * _node );

	void update_movie_composition( const aeMovieInstance * _instance, const aeMovieNode * _node, float _timing );
	
	typedef struct aeMovieRender
	{
		uint32_t vertexCount;
		uint32_t indexCount;

		float * position;
		float * uv;

		uint16_t * indices;

		float r;
		float g;
		float b;
		float a;
	} aeMovieRender;

	void compute_movie_vertices( const aeMovieInstance * _instance, const aeMovieNode * _node, const aeMovieLayerData * _layer, aeMovieRender * _render );
	
#ifdef __cplusplus
}
#endif

#endif