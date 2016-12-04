#ifndef MOVIE_MOVIE_SKELETON_H_
#define MOVIE_MOVIE_SKELETON_H_

#	include "movie_type.h"
#	include "movie_instance.h"
#	include "movie_resource.h"
#	include "movie_data.h"

typedef struct aeMovieComposition aeMovieComposition;

typedef struct aeMovieSkeleton
{
	aeMovieComposition * base;

	aeMovieComposition * animations[8];
} aeMovieSkeleton;

aeMovieSkeleton * ae_movie_create_skeleton( aeMovieComposition * _base );
ae_bool_t ae_movie_skeleton_add_animation( aeMovieSkeleton * _skeleton, aeMovieComposition * _animation );

void ae_movie_destroy_skeleton( const aeMovieSkeleton * _skeleton );

#endif