#	include "movie/movie_node.h"

#	include "memory.h"

//////////////////////////////////////////////////////////////////////////
aeMovieNode * create_movie_node( const aeMovieInstance * _instance, const aeMovieData * _data, const aeMovieCompositionData * _composition )
{
	aeMovieNode * node = NEW( _instance, aeMovieNode );

	node->data = _data;
	node->composition = _composition;



	return node;
}
//////////////////////////////////////////////////////////////////////////
void delete_movie_node( const aeMovieInstance * _instance, const aeMovieNode * _node )
{
	DELETE( _instance, _node );
}
//////////////////////////////////////////////////////////////////////////