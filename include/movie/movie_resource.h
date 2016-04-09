#ifndef MOVIE_MOVIE_RESOURCE_H_
#define MOVIE_MOVIE_RESOURCE_H_

#	include <movie/movie_type.h>

#ifdef __cplusplus
extern "C" {
#endif

	enum
	{
		AE_MOVIE_RESOURCE_SOCKET_SHAPE = 2,
		AE_MOVIE_RESOURCE_SOCKET_IMAGE = 3,
		AE_MOVIE_RESOURCE_SOLID = 4,
		AE_MOVIE_RESOURCE_VIDEO = 5,
		AE_MOVIE_RESOURCE_SOUND = 6,
		AE_MOVIE_RESOURCE_IMAGE = 7,
		AE_MOVIE_RESOURCE_IMAGE_SEQUENCE = 8,
		AE_MOVIE_RESOURCE_ASTRALAX = 9,
		__AE_MOVIE_RESOURCE_END__
	};

	typedef struct aeMovieResource
	{
		uint8_t type;

		void * data;

	} aeMovieResource;

	typedef struct
	{
		aeMovieResource base;

		ae_string_t name;

	} aeMovieResourceInternal;

	typedef struct
	{
		aeMovieResource base;

		aeMoviePolygon * polygons;

	} aeMovieResourceSocketShape;

	typedef struct
	{
		aeMovieResource base;

		ae_string_t path;

	} aeMovieResourceSocketImage;
	
	typedef struct
	{
		aeMovieResource base;

		float width;
		float height;
		float r;
		float g;
		float b;

	} aeMovieResourceSolid;

	typedef struct
	{
		aeMovieResource base;

		ae_string_t path;

		uint8_t alpha;
		float frameRate;
		float duration;

	} aeMovieResourceVideo;

	typedef struct
	{
		aeMovieResource base;

		ae_string_t path;

		float duration;

	} aeMovieResourceSound;

	typedef struct
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

	typedef struct
	{
		aeMovieResource base;

		float frameDuration;

		uint32_t image_count;
		aeMovieResourceImage ** images;

	} aeMovieResourceImageSequence;

	typedef struct
	{
		aeMovieResource base;

		ae_string_t path;

		uint32_t atlas_count;
		aeMovieResourceImage ** atlases;

	} aeMovieResourceAstralax;

#ifdef __cplusplus
}
#endif

#endif