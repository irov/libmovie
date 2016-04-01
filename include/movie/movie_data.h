#ifndef MOVIE_MOVIE_DATA_H_
#define MOVIE_MOVIE_DATA_H_

#	include <movie/movie_type.h>
#	include <movie/movie_instance.h>
#	include <movie/movie_resource.h>

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct aeMovieLayerData
	{
		ae_string_t name;

		uint32_t frame_count;

	} aeMovieLayerData;
	
	typedef enum
	{
		AE_MOVIE_COMPOSITION_LOOP_SEGMENT = 0x00000001,
		AE_MOVIE_COMPOSITION_ANCHOR_POINT = 0x00000002,
		AE_MOVIE_COMPOSITION_OFFSET_POINT = 0x00000004,
		AE_MOVIE_COMPOSITION_BOUNDS = 0x00000008,
		__AE_MOVIE_COMPOSITION_FLAGS__
	} aeMovieCompositionFlag;

	typedef struct aeMovieCompositionData
	{
		ae_string_t name;
		
		float width;
		float height;

		float frameDuration;
		float duration;

		uint32_t flags;

		float loopSegment[2];
		float anchorPoint[3];
		float offsetPoint[3];
		float bounds[4];

		aeMovieLayerData * layers;
	} aeMovieCompositionData;

	typedef struct aeMovieData
	{
		ae_string_t name;

		float duration;

		aeMovieResource ** resources;
		aeMovieCompositionData * compositions;
	} aeMovieData;

	aeMovieData * create_movie_data( const aeMovieInstance * _instance );
	void delete_movie_data( const aeMovieInstance * _instance, aeMovieData * _movie );

	aeMovieResult load_movie_data( const aeMovieInstance * _instance, aeMovieData * _movie, const aeMovieStream * _stream );

#ifdef __cplusplus
}
#endif

#endif