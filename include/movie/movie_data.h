#ifndef MOVIE_MOVIE_DATA_H_
#define MOVIE_MOVIE_DATA_H_

#	include "movie_type.h"
#	include "movie_instance.h"
#	include "movie_resource.h"

typedef struct
{
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
	aeMovieBezierWarp immutable_bezier_warp;

	aeMovieBezierWarp * bezier_warps;

} aeMovieLayerBezierWarp;

typedef struct
{
	ae_bool_t immutable_r;
	ae_bool_t immutable_g;
	ae_bool_t immutable_b;

	float immutable_color_vertex_r;
	float immutable_color_vertex_g;
	float immutable_color_vertex_b;

	ae_color_t * color_vertites_r;
	ae_color_t * color_vertites_g;
	ae_color_t * color_vertites_b;

} aeMovieLayerColorVertex;

typedef struct
{
	ae_bool_t immutable;
	aeMoviePolygon immutable_polygon;

	aeMoviePolygon * polygons;

} aeMovieLayerPolygon;

typedef enum aeMovieLayerTypeEnum
{
	AE_MOVIE_LAYER_TYPE_ANY = 0,
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

	struct aeMovieCompositionData * composition_data;

	ae_bool_t is_track_matte;
	ae_bool_t has_track_matte;
	struct aeMovieLayerData * track_matte_layer;

	uint32_t frame_count;

	const aeMovieLayerTimeremap * timeremap;
	const aeMovieLayerMesh * mesh;
	const aeMovieLayerBezierWarp * bezier_warp;
	const aeMovieLayerColorVertex * color_vertex;
	const aeMovieLayerPolygon * polygon;

	const aeMovieResource * resource;
	const struct aeMovieCompositionData * sub_composition_data;

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

	ae_bool_t master;

	float width;
	float height;

	float duration;

	float frameDuration;
	float frameDurationInv;

	uint32_t frameCount;

	uint32_t flags;

	ae_vector2_t loop_segment;
	ae_vector3_t anchor_point;
	ae_vector3_t offset_point;
	ae_vector4_t bounds;

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
	const aeMovieCompositionData * compositions;
} aeMovieData;

aeMovieData * ae_create_movie_data( const aeMovieInstance * _instance );
void ae_delete_movie_data( const aeMovieData * _movieData );

typedef void * (*ae_movie_data_resource_provider_t)(const aeMovieResource * _resource, void * _data);

aeMovieStream * ae_create_movie_stream( const aeMovieInstance * _instance, ae_movie_stream_memory_read_t _read, ae_movie_stream_memory_copy_t _copy, void * _data );
void ae_delete_movie_stream( aeMovieStream * _stream );

ae_result_t ae_load_movie_data( aeMovieData * _movieData, aeMovieStream * _stream, ae_movie_data_resource_provider_t _provider, void * _data );

typedef ae_bool_t( *ae_movie_data_tream_image_resource_t )(const aeMovieResourceImage * _imageResource, float * base_width, float * base_height, float * trim_width, float * trim_height, float * offset_x, float * offset_y, void * _data);

void ae_trim_image_resources( aeMovieData * _movieData, ae_movie_data_tream_image_resource_t _provider, void * _data );

const aeMovieCompositionData * ae_get_movie_composition_data( const aeMovieData * _movieData, const ae_char_t * _name );

float ae_get_movie_composition_data_duration( const aeMovieCompositionData * _compositionData );

ae_bool_t ae_get_movie_composition_data_master( const aeMovieCompositionData * _compositionData );

uint32_t ae_get_movie_composition_data_count( const aeMovieData * _movieData );
const aeMovieCompositionData * ae_get_movie_composition_data_by_index( const aeMovieData * _movieData, uint32_t _index );

uint32_t ae_get_composition_data_event_count( const aeMovieCompositionData * _compositionData );
const ae_char_t * ae_get_composition_data_event_name( const aeMovieCompositionData * _compositionData, uint32_t _index );

#endif