#ifndef MOVIE_MOVIE_RESOURCE_H_
#define MOVIE_MOVIE_RESOURCE_H_

#	include "movie/movie_type.h"

typedef struct aeMovieResourceSolid
{
	AE_MOVIE_RESOURCE_BASE();

	float width;
	float height;
	float r;
	float g;
	float b;

} aeMovieResourceSolid;

typedef struct aeMovieResourceVideo
{
	AE_MOVIE_RESOURCE_BASE();

	ae_string_t path;

	float width;
	float height;

	uint32_t alpha;
	float frameRate;
	float duration;

} aeMovieResourceVideo;

typedef struct aeMovieResourceSound
{
	AE_MOVIE_RESOURCE_BASE();

	ae_string_t path;

	float duration;

} aeMovieResourceSound;

typedef struct aeMovieResourceImage
{
	AE_MOVIE_RESOURCE_BASE();

	ae_string_t path;

	ae_bool_t premultiplied;

	float base_width;
	float base_height;
	float trim_width;
	float trim_height;
	float offset_x;
	float offset_y;

	const aeMovieMesh * mesh;

} aeMovieResourceImage;

typedef struct aeMovieResourceSequence
{
	AE_MOVIE_RESOURCE_BASE();

	float frameDurationInv;

	uint32_t image_count;
	const aeMovieResourceImage * const * images;

} aeMovieResourceSequence;

typedef struct aeMovieResourceParticle
{
	AE_MOVIE_RESOURCE_BASE();

	ae_string_t path;

} aeMovieResourceParticle;

typedef struct aeMovieResourceSlot
{
	AE_MOVIE_RESOURCE_BASE();

	float width;
	float height;

} aeMovieResourceSlot;

#	undef AE_MOVIE_RESOURCE_BASE

#endif