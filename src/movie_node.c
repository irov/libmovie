#	include "movie/movie_node.h"

#	include "memory.h"

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
aeMovieNode * create_movie_node( const aeMovieInstance * _instance, const aeMovieData * _data, const aeMovieCompositionData * _composition )
{
	aeMovieNode * node = NEW( _instance, aeMovieNode );

	node->data = _data;
	node->composition = _composition;

	node->timing = 0.f;

	return node;
}
//////////////////////////////////////////////////////////////////////////
void delete_movie_node( const aeMovieInstance * _instance, const aeMovieNode * _node )
{
	DELETE( _instance, _node );
}
//////////////////////////////////////////////////////////////////////////
void update_movie_node( const aeMovieInstance * _instance, const aeMovieNode * _node )
{

}
//////////////////////////////////////////////////////////////////////////
void compute_movie_vertices( const aeMovieInstance * _instance, const aeMovieNode * _node, const aeMovieLayerData * _layer, aeMovieRender * _render )
{
	for( const aeMovieCompositionData
		*it_composition = _movie->compositions,
		*it_composition_end = _movie->compositions + _movie->composition_count;
	it_composition != it_composition_end;
	++it_composition )
	{
		const aeMovieCompositionData * composition = it_composition;

		if( ae_strcmp( composition->name, _name ) != 0 )
		{
			continue;
		}

		return composition;
	}
}
//////////////////////////////////////////////////////////////////////////