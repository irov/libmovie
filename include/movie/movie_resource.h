#ifndef MOVIE_MOVIE_RESOURCE_H_
#define MOVIE_MOVIE_RESOURCE_H_

#	include <movie/movie_type.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef enum aeMovieResourceTypeEnum
	{
		AE_MOVIE_RESOURCE_NONE = 0,
		AE_MOVIE_RESOURCE_SOCKET = 2,
		AE_MOVIE_RESOURCE_SOLID = 4,
		AE_MOVIE_RESOURCE_VIDEO = 5,
		AE_MOVIE_RESOURCE_SOUND = 6,
		AE_MOVIE_RESOURCE_IMAGE = 7,
		AE_MOVIE_RESOURCE_SEQUENCE = 8,
		AE_MOVIE_RESOURCE_PARTICLE = 9,
		__AE_MOVIE_RESOURCE_END__
	} aeMovieResourceTypeEnum;

#	define AE_MOVIE_RESOURCE_BASE()\
	uint8_t type;\
	void * data

	typedef struct aeMovieResource
	{
		AE_MOVIE_RESOURCE_BASE();
		
	} aeMovieResource;

	typedef struct
	{
		AE_MOVIE_RESOURCE_BASE();

		aeMoviePolygon polygon;

	} aeMovieResourceSocket;

	typedef struct
	{
		AE_MOVIE_RESOURCE_BASE();

		float width;
		float height;
		float r;
		float g;
		float b;

	} aeMovieResourceSolid;

	typedef struct
	{
		AE_MOVIE_RESOURCE_BASE();

		ae_string_t path;

		float width;
		float height;

		uint8_t alpha;
		float frameRate;
		float duration;

	} aeMovieResourceVideo;

	typedef struct
	{
		AE_MOVIE_RESOURCE_BASE();

		ae_string_t path;

		float duration;

	} aeMovieResourceSound;

	typedef struct
	{
		AE_MOVIE_RESOURCE_BASE();

		ae_string_t path;

		float base_width;
		float base_height;
		float trim_width;
		float trim_height;
		float offset_x;
		float offset_y;

	} aeMovieResourceImage;

	typedef struct
	{
		AE_MOVIE_RESOURCE_BASE();

		float frameDuration;

		uint32_t image_count;
		aeMovieResourceImage ** images;

	} aeMovieResourceSequence;

	typedef struct
	{
		AE_MOVIE_RESOURCE_BASE();

		ae_string_t path;
		
	} aeMovieResourceParticle;

#	undef AE_MOVIE_RESOURCE_BASE

#ifdef __cplusplus
}
#endif

#endif