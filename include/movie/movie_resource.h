#ifndef MOVIE_MOVIE_RESOURCE_H_
#define MOVIE_MOVIE_RESOURCE_H_

#	include <movie/movie_type.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct aeMovieResource
	{
		uint8_t type;

	} aeMovieResource;

	typedef struct aeMovieResourceInternal
	{
		aeMovieResource base;

		ae_string_t name;

	} aeMovieResourceInternal;

	typedef struct aeMovieResourceSocketShape
	{
		aeMovieResource base;

		aeMoviePolygon * polygons;

	} aeMovieResourceSocketShape;

	typedef struct aeMovieResourceSocketImage
	{
		aeMovieResource base;

		ae_string_t path;

	} aeMovieResourceSocketImage;
	
	typedef struct aeMovieResourceSolid
	{
		aeMovieResource base;

		float width;
		float height;
		float r;
		float g;
		float b;

	} aeMovieResourceSolid;

	typedef struct aeMovieResourceVideo
	{
		aeMovieResource base;

		ae_string_t path;

		uint8_t alpha;
		float frameRate;
		float duration;

	} aeMovieResourceVideo;

	typedef struct aeMovieResourceSound
	{
		aeMovieResource base;

		ae_string_t path;

		float duration;

	} aeMovieResourceSound;

	typedef struct aeMovieResourceImage
	{
		aeMovieResource base;

		ae_string_t path;

		float base_width;
		float base_height;
		float trim_width;
		float trim_height;
		float trim_offset_x;
		float trim_offset_y;

	} aeMovieResourceImage;

	typedef struct aeMovieResourceImageSequence
	{
		aeMovieResource base;

		float frameDuration;

		uint32_t count;
		aeMovieResourceImage * images;

	} aeMovieResourceImageSequence;

#ifdef __cplusplus
}
#endif

#endif