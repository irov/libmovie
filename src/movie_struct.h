#	ifndef MOVIE_STRUCT_H_
#	define MOVIE_STRUCT_H_

#	include "movie/movie_type.h"
#	include "movie/movie_instance.h"
#	include "movie/movie_node.h"

//////////////////////////////////////////////////////////////////////////
struct aeMovieInstance
{
	uint32_t hashmask[5];

	ae_movie_alloc_t memory_alloc;
	ae_movie_alloc_n_t memory_alloc_n;
	ae_movie_free_t memory_free;
	ae_movie_free_n_t memory_free_n;	
	ae_movie_strncmp_t strncmp;
	ae_movie_logger_t logger;
	void * instance_data;

    ae_vector2_t sprite_uv[4];
	uint16_t sprite_indices[6];	

    ae_vector2_t bezier_warp_uv[AE_MOVIE_BEZIER_WARP_GRID_VERTEX_COUNT];
	uint16_t bezier_warp_indices[AE_MOVIE_BEZIER_WARP_GRID_INDICES_COUNT];
};
//////////////////////////////////////////////////////////////////////////
struct aeMovieStream
{
	const aeMovieInstance * instance;

	ae_movie_stream_memory_read_t memory_read;
	ae_movie_stream_memory_copy_t memory_copy;
	void * data;

#	ifdef AE_MOVIE_STREAM_CACHE
	size_t carriage;
	size_t capacity;
	size_t reading;

	uint8_t buff[AE_MOVIE_STREAM_CACHE_BUFFER_SIZE];
#	endif
};
//////////////////////////////////////////////////////////////////////////
struct aeMovieCompositionAnimation
{
	ae_bool_t play;
	ae_bool_t pause;
	ae_bool_t interrupt;
	ae_bool_t loop;
			
	float time;

	float loop_segment_begin;
	float loop_segment_end;

	float work_area_begin;
	float work_area_end;
};
//////////////////////////////////////////////////////////////////////////
struct aeMovieSubComposition
{
	const aeMovieLayerData * layer;
	const aeMovieCompositionData * composition_data;

	struct aeMovieCompositionAnimation * animation;
};
//////////////////////////////////////////////////////////////////////////
struct aeMovieNode
{
	const aeMovieLayerData * layer;

	struct aeMovieNode * relative;
    struct aeMovieNode * track_matte;

	const aeMovieSubComposition * subcomposition;

	float start_time;
	float in_time;
	float out_time;

	float stretch;
	float current_time;

	ae_bool_t active;
	ae_bool_t ignore;
	ae_bool_t enable;

	uint32_t animate;

	uint32_t matrix_revision;
	ae_matrix4_t matrix;

	float composition_opactity;
	float opacity;

    float composition_r;
    float composition_g;
    float composition_b;
    float r;    
    float g;
    float b;

	aeMovieBlendMode blend_mode;

	void * camera_data;
	void * element_data;
	void * track_matte_data;
};
//////////////////////////////////////////////////////////////////////////
struct aeMovieComposition
{
	const aeMovieData * movie_data;
	const aeMovieCompositionData * composition_data;

	struct aeMovieCompositionAnimation * animation;

	uint32_t * update_revision;

	ae_bool_t interpolate;

	uint32_t node_count;
	aeMovieNode * nodes;

	uint32_t subcomposition_count;
	aeMovieSubComposition * subcompositions;

	aeMovieCompositionProviders providers;
	void * provider_data;
};
//////////////////////////////////////////////////////////////////////////
struct aeMovieCompositionData
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
	const aeMovieLayerData * layers;
};
//////////////////////////////////////////////////////////////////////////
struct aeMovieData
{
	const aeMovieInstance * instance;
	
	ae_string_t name;

	uint32_t resource_count;
	const aeMovieResource * const * resources;

	uint32_t composition_count;
	const aeMovieCompositionData * compositions;
};
//////////////////////////////////////////////////////////////////////////
struct aeMovieLayerData
{
	ae_string_t name;

	uint32_t index;
	aeMovieLayerTypeEnum type;

	ae_bool_t renderable;

	const struct aeMovieCompositionData * composition_data;

	ae_bool_t is_track_matte;
	ae_bool_t has_track_matte;
	const struct aeMovieLayerData * track_matte_layer;

	uint32_t frame_count;

	const aeMovieLayerTimeremap * timeremap;
	const aeMovieLayerMesh * mesh;
	const aeMovieLayerBezierWarp * bezier_warp;
	const aeMovieLayerColorVertex * color_vertex;
	const aeMovieLayerPolygon * polygon;

	const aeMovieResource * resource;
	const aeMovieCompositionData * sub_composition_data;

	uint32_t parent_index;

	ae_bool_t reverse_time;
	float start_time;
	float in_time;
	float out_time;

	uint32_t blend_mode;
	ae_bool_t threeD;
	uint32_t params;

	uint32_t play_count;

	float stretch;

	const struct aeMovieLayerTransformation * transformation;
};
//////////////////////////////////////////////////////////////////////////
struct aeMovieLayerTimeremap
{
	const float * times;

};
//////////////////////////////////////////////////////////////////////////
struct aeMovieLayerMesh
{
	ae_bool_t immutable;
	aeMovieMesh immutable_mesh;

	const aeMovieMesh * meshes;

};
//////////////////////////////////////////////////////////////////////////
struct aeMovieLayerBezierWarp
{
	ae_bool_t immutable;
	aeMovieBezierWarp immutable_bezier_warp;

	const aeMovieBezierWarp * bezier_warps;

};
//////////////////////////////////////////////////////////////////////////
struct aeMovieLayerColorVertex
{
	ae_bool_t immutable_r;
	ae_bool_t immutable_g;
	ae_bool_t immutable_b;

	float immutable_color_vertex_r;
	float immutable_color_vertex_g;
	float immutable_color_vertex_b;

	const ae_color_t * color_vertites_r;
	const ae_color_t * color_vertites_g;
	const ae_color_t * color_vertites_b;

};
//////////////////////////////////////////////////////////////////////////
struct aeMovieLayerPolygon
{
	ae_bool_t immutable;
	aeMoviePolygon immutable_polygon;

	const aeMoviePolygon * polygons;

};
//////////////////////////////////////////////////////////////////////////
#	endif
