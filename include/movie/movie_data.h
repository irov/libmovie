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

	typedef enum aeMoviePropertyImmutableEnum
	{
		AE_MOVIE_IMMUTABLE_ANCHOR_POINT_X = 0x0001,
		AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Y = 0x0002,
		AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Z = 0x0004,
		AE_MOVIE_IMMUTABLE_POSITION_X = 0x0008,
		AE_MOVIE_IMMUTABLE_POSITION_Y = 0x0010,
		AE_MOVIE_IMMUTABLE_POSITION_Z = 0x0020,
		AE_MOVIE_IMMUTABLE_ROTATION_X = 0x0040,
		AE_MOVIE_IMMUTABLE_ROTATION_Y = 0x0080,
		AE_MOVIE_IMMUTABLE_ROTATION_Z = 0x0100,
		AE_MOVIE_IMMUTABLE_SCALE_X = 0x0400,
		AE_MOVIE_IMMUTABLE_SCALE_Y = 0x0800,
		AE_MOVIE_IMMUTABLE_SCALE_Z = 0x1000,
		AE_MOVIE_IMMUTABLE_OPACITY = 0x2000,
		__AE_MOVIE_IMMUTABLE_END__
	} aeMoviePropertyImmutableEnum;

	typedef enum aeMovieLayerTypeEnum
	{
		AE_MOVIE_LAYER_TYPE_MOVIE = 1,
		AE_MOVIE_LAYER_TYPE_SPRITE = 5,
		AE_MOVIE_LAYER_TYPE_EVENT = 7,
		AE_MOVIE_LAYER_TYPE_SOCKET_SHAPE = 8,
		AE_MOVIE_LAYER_TYPE_SOCKET_IMAGE = 9,
		AE_MOVIE_LAYER_TYPE_SLOT = 11,
		AE_MOVIE_LAYER_TYPE_NULL = 12,
		AE_MOVIE_LAYER_TYPE_SPRITE_SOLID = 14,
		AE_MOVIE_LAYER_TYPE_SPRITE_SEQUENCE = 15,
		AE_MOVIE_LAYER_TYPE_VIDEO = 16,
		AE_MOVIE_LAYER_TYPE_SOUND = 17,
		AE_MOVIE_LAYER_TYPE_ASTRALAX = 18,
		AE_MOVIE_LAYER_TYPE_IMAGE = 20,
		AE_MOVIE_LAYER_TYPE_SUB_MOVIE = 21,
	} aeMovieLayerTypeEnum;

	typedef struct aeMovieLayerData
	{
		ae_string_t name;

		uint32_t index;
		uint8_t type;

		struct aeMovieCompositionData * composition;

		uint32_t frame_count;

		aeMovieLayerTimeremap * timeremap;
		aeMovieLayerMesh * mesh;
		aeMovieLayerPolygon * polygon;
		aeMovieLayerViewportMatte * viewport_matte;

		struct aeMovieResource * resource;
		struct aeMovieCompositionData * sub_composition;

		uint32_t parent_index;

		float start_time;
		float in_time;
		float out_time;

		uint8_t blend_mode;		
		uint32_t params;

		uint32_t play_count;

		float stretch;
				
		uint32_t immutable_property_mask;

		float immuttable_anchor_point_x;
		float immuttable_anchor_point_y;
		float immuttable_anchor_point_z;
		float immuttable_position_x;
		float immuttable_position_y;
		float immuttable_position_z;
		float immuttable_rotation_x;
		float immuttable_rotation_y;
		float immuttable_rotation_z;
		float immuttable_scale_x;
		float immuttable_scale_y;
		float immuttable_scale_z;
		float immuttable_opacity;

		float * property_anchor_point_x;
		float * property_anchor_point_y;
		float * property_anchor_point_z;
		float * property_position_x;
		float * property_position_y;
		float * property_position_z;
		float * property_rotation_x;
		float * property_rotation_y;
		float * property_rotation_z;
		float * property_scale_x;
		float * property_scale_y;
		float * property_scale_z;
		float * property_opacity;
	} aeMovieLayerData;

	typedef struct 
	{
		float camera_position[3];
		float camera_fov;
		float camera_aspect;
		float camera_width;
		float camera_height;
	} aeMovieCameraData;
	
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

		aeMovieCameraData * camera;

		uint32_t layer_count;
		aeMovieLayerData * layers;
	} aeMovieCompositionData;

	typedef struct aeMovieData
	{
		ae_string_t name;

		float duration;

		uint32_t resource_count;
		uint32_t composition_count;

		aeMovieResource ** resources;
		aeMovieCompositionData * compositions;
	} aeMovieData;

	aeMovieData * create_movie_data( const aeMovieInstance * _instance );
	void delete_movie_data( const aeMovieInstance * _instance, const aeMovieData * _movie );

	typedef void * (*movie_data_resource_provider_t)(aeMovieResource * _resource, void * _data);

	aeMovieResult load_movie_data( const aeMovieInstance * _instance, const aeMovieStream * _stream, aeMovieData * _movie, movie_data_resource_provider_t _provider, void * _data );
	
	const aeMovieCompositionData * get_movie_composition_data( const aeMovieData * _movie, const char * _name );
	uint32_t get_movie_composition_data_node_count( const aeMovieData * _movie, const aeMovieCompositionData * _compositionData );

#ifdef __cplusplus
}
#endif

#endif