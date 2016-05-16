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

	typedef enum aeMovieLayerTypeEnum
	{
		AE_MOVIE_LAYER_TYPE_MOVIE = 1,
		AE_MOVIE_LAYER_TYPE_EVENT = 7,
		AE_MOVIE_LAYER_TYPE_SOCKET = 8,
		AE_MOVIE_LAYER_TYPE_SHAPE = 9,
		AE_MOVIE_LAYER_TYPE_SLOT = 11,
		AE_MOVIE_LAYER_TYPE_NULL = 12,
		AE_MOVIE_LAYER_TYPE_SOLID = 14,
		AE_MOVIE_LAYER_TYPE_SEQUENCE = 15,
		AE_MOVIE_LAYER_TYPE_VIDEO = 16,
		AE_MOVIE_LAYER_TYPE_SOUND = 17,
		AE_MOVIE_LAYER_TYPE_PARTICLE = 18,
		AE_MOVIE_LAYER_TYPE_IMAGE = 20,
		AE_MOVIE_LAYER_TYPE_SUB_MOVIE = 21,
	} aeMovieLayerTypeEnum;

	typedef enum aeMovieLayerParamEnum
	{
		AE_MOVIE_LAYER_PARAM_LOOP = 0x00000008,
		AE_MOVIE_LAYER_PARAM_SWITCH = 0x00000010,
		__AE_MOVIE_LAYER_PARAM_END__
	} aeMovieLayerParamEnum;

	typedef struct aeMovieLayerData
	{
		ae_string_t name;

		uint32_t index;
		uint8_t type;
		
		ae_bool_t renderable;

		struct aeMovieCompositionData * composition;

		ae_bool_t is_track_matte;
		
		ae_bool_t has_track_matte;

		uint32_t frame_count;

		aeMovieLayerTimeremap * timeremap;
		aeMovieLayerMesh * mesh;
		
		const aeMovieResource * resource;
		struct aeMovieCompositionData * sub_composition;

		uint32_t parent_index;

		ae_bool_t reverse_time;
		float start_time;
		float in_time;
		float out_time;

		uint8_t blend_mode;		
		ae_bool_t threeD;
		uint32_t params;
		
		uint32_t play_count;

		float stretch;
				
		struct aeMovieLayerTransformation * transformation;
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

		uint32_t frameCount;

		uint32_t flags;

		float loopSegment[2];
		float anchorPoint[3];
		float offsetPoint[3];
		float bounds[4];

		ae_bool_t has_threeD;

		float cameraZoom;

		uint32_t layer_count;
		aeMovieLayerData * layers;
	} aeMovieCompositionData;

	typedef struct aeMovieData
	{
		const aeMovieInstance * instance;

		ae_string_t name;
		
		uint32_t resource_count;
		const aeMovieResource ** resources;

		uint32_t composition_count;
		aeMovieCompositionData * compositions;
	} aeMovieData;

	aeMovieData * ae_create_movie_data( const aeMovieInstance * _instance );
	void ae_delete_movie_data( const aeMovieData * _movie );

	typedef void * (*ae_movie_data_resource_provider_t)(const aeMovieResource * _resource, void * _data);

	aeMovieResult ae_load_movie_data( aeMovieData * _movie, const aeMovieStream * _stream, ae_movie_data_resource_provider_t _provider, void * _data );
	
	const aeMovieCompositionData * ae_get_movie_composition_data( const aeMovieData * _movie, const char * _name );	
	
#ifdef __cplusplus
}
#endif

#endif