#ifndef MOVIE_MOVIE_DATA_H_
#define MOVIE_MOVIE_DATA_H_

#	include <movie/movie_type.h>
#	include <movie/movie_instance.h>
#	include <movie/movie_resource.h>

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct
	{
		ae_bool_t immutable;
		float immutable_time;

		float * times;

	} aeMovieLayerTimeremap;

	typedef struct
	{
		ae_bool_t immutable;
		aeMovieMesh immutable_mesh;

		aeMovieMesh * meshes;
		
	} aeMovieLayerMesh;

	typedef struct
	{
		ae_bool_t immutable;
		aeMoviePolygon immutable_polygon;

		aeMoviePolygon * polygons;

	} aeMovieLayerPolygon;

	typedef struct
	{
		ae_bool_t immutable;
		aeMovieViewport immutable_viewport;

		aeMovieViewport * viewports;

	} aeMovieLayerViewportMatte;

	typedef struct aeMovieLayerData
	{
		ae_string_t name;

		uint32_t index;
		uint32_t type;

		ae_bool_t immutable;
		uint32_t frame_count;		

		aeMovieLayerTimeremap * timeremap;
		aeMovieLayerMesh * mesh;
		aeMovieLayerPolygon * polygon;
		aeMovieLayerViewportMatte * viewport_matte;

		aeMovieResource * resource;

		struct aeMovieLayerData * parent;

		float start_time;
		float in_time;
		float out_time;

		uint32_t blend_mode;		
		uint32_t flags;

		uint32_t play_count;

		float stretch;
				
		aeMovieLayerProperty properties;

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

	aeMovieResult load_movie_data( const aeMovieInstance * _instance, const aeMovieStream * _stream, aeMovieData * _movie );

#ifdef __cplusplus
}
#endif

#endif