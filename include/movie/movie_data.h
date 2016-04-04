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

	static const uint16_t AE_MOVIE_IMMUTABLE_ANCHOR_POINT_X = 0x0001;
	static const uint16_t AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Y = 0x0002;
	static const uint16_t AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Z = 0x0004;
	static const uint16_t AE_MOVIE_IMMUTABLE_POSITION_X = 0x0008;
	static const uint16_t AE_MOVIE_IMMUTABLE_POSITION_Y = 0x0010;
	static const uint16_t AE_MOVIE_IMMUTABLE_POSITION_Z = 0x0020;
	static const uint16_t AE_MOVIE_IMMUTABLE_ROTATION_X = 0x0040;
	static const uint16_t AE_MOVIE_IMMUTABLE_ROTATION_Y = 0x0080;
	static const uint16_t AE_MOVIE_IMMUTABLE_ROTATION_Z = 0x0100;
	static const uint16_t AE_MOVIE_IMMUTABLE_SCALE_X = 0x0400;
	static const uint16_t AE_MOVIE_IMMUTABLE_SCALE_Y = 0x0800;
	static const uint16_t AE_MOVIE_IMMUTABLE_SCALE_Z = 0x1000;
	static const uint16_t AE_MOVIE_IMMUTABLE_OPACITY = 0x2000;

	typedef struct aeMovieLayerData
	{
		ae_string_t name;

		uint32_t index;
		uint32_t type;

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
				
		uint16_t immutable_property_mask;

		float immuttable_anchor_point_x;
		float immuttable_anchor_point_y;
		float immuttable_anchor_point_z;
		float immuttable_position_x;
		float immuttable_position_y;
		float immuttable_position_z;
		float immuttable_rotation_x;
		float immuttable_rotation_y;
		float immuttable_rotation_z;
		float immuttable_rotation_w;
		float immuttable_scale_x;
		float immuttable_scale_y;
		float immuttable_scale_z;
		float immuttable_opacity;
		float immuttable_volume;

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
		float * property_volume;
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