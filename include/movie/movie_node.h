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
		const aeMovieData * data;
		const aeMovieData * composition;

	} aeMovieNode;

	aeMovieNode * create_movie_node( const aeMovieInstance * _instance, const aeMovieData * _data, const aeMovieCompositionData * _composition );
	void delete_movie_node( const aeMovieInstance * _instance, const aeMovieNode * _node );
	
#ifdef __cplusplus
}
#endif

#endif