#	include "movie/movie_skeleton.h"

#	include "movie_memory.h"
#	include "movie_math.h"
#	include "movie_struct.h"

static ae_bool_t __test_movie_skeleton_base( aeMovieComposition * _base )
{
	float duration = _base->composition_data->duration;

	for( const aeMovieNode
		*it = _base->nodes,
		*it_end = _base->nodes + _base->node_count;
	it != it_end;
	++it )
	{
		const aeMovieNode * node = it;

		if( equal_f_z( node->in_time ) == AE_FALSE )
		{
			return AE_FALSE;
		}

		if( equal_f_f( node->out_time, duration ) == AE_FALSE )
		{
			return AE_FALSE;
		}

		if( equal_f_z( node->start_time ) == AE_FALSE )
		{
			return AE_FALSE;
		}
	}

	return AE_TRUE;
}

aeMovieSkeleton * ae_movie_create_skeleton( aeMovieComposition * _base )
{
	if( _base == AE_NULL )
	{
		return AE_NULL;
	}

	if( __test_movie_skeleton_base( _base ) == AE_FALSE )
	{
		return AE_NULL;
	}

	const aeMovieInstance * instance = _base->movie_data->instance;

	aeMovieSkeleton * skeleton = NEW( instance, aeMovieSkeleton );

	skeleton->base = _base;

	for( uint32_t i = 0; i != 8; ++i )
	{
		skeleton->animations[i] = AE_NULL;
	}

	return skeleton;
}

static ae_bool_t __test_movie_skeleton_animation( aeMovieComposition * _base, aeMovieComposition * _animation )
{
	if( _base->node_count != _animation->node_count )
	{
		return AE_FALSE;
	}

	const aeMovieInstance * instance = _base->movie_data->instance;

	for( const aeMovieNode
		*it_base = _base->nodes,
		*it_base_end = _base->nodes + _base->node_count,
		*it_animation = _animation->nodes,
		*it_animation_end = _animation->nodes + _animation->node_count;
	it_base != it_base_end;
	++it_base, ++it_animation )
	{
		const aeMovieNode * base_node = it_base;
		const aeMovieNode * animation_node = it_animation;

		if( base_node->layer->type != animation_node->layer->type )
		{
			return AE_FALSE;
		}

		if( instance->strncmp( instance->instance_data, base_node->layer->name, animation_node->layer->name, AE_MOVIE_MAX_LAYER_NAME ) == AE_FALSE )
		{
			return AE_FALSE;
		}
	}

	return AE_TRUE;
}

static uint32_t __movie_skeleton_find_free_animation_place( aeMovieSkeleton * _skeleton )
{
	for( uint32_t i = 0; i != 8; ++i )
	{
		if( _skeleton->animations[i] != AE_NULL )
		{
			continue;
		}

		return i;
	}

	return (uint32_t)-1;
}

ae_bool_t ae_movie_skeleton_add_animation( aeMovieSkeleton * _skeleton, aeMovieComposition * _animation )
{
	if( __test_movie_skeleton_base( _animation ) == AE_FALSE )
	{
		return AE_FALSE;
	}

	aeMovieComposition * base = _skeleton->base;

	if( __test_movie_skeleton_animation( base, _animation ) == AE_FALSE )
	{
		return AE_FALSE;
	}

	uint32_t index = __movie_skeleton_find_free_animation_place( _skeleton );

	if( index == (uint32_t)-1 )
	{
		return AE_FALSE;
	}

	_skeleton->animations[index] = _animation;

	return AE_TRUE;
}

void ae_movie_destroy_skeleton( const aeMovieSkeleton * _skeleton )
{
	DELETE( _skeleton->base->movie_data->instance, _skeleton );
}
