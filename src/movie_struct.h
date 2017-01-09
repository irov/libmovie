#	ifndef MOVIE_STRUCT_H_
#	define MOVIE_STRUCT_H_

#	include "movie/movie_type.h"
#	include "movie/movie_instance.h"
#	include "movie/movie_node.h"

//////////////////////////////////////////////////////////////////////////
typedef struct aeMovieInstance
{	
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
} aeMovieInstance;
//////////////////////////////////////////////////////////////////////////
typedef struct aeMovieStream
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
} aeMovieStream;
//////////////////////////////////////////////////////////////////////////
typedef struct aeMovieCompositionAnimation
{
	ae_bool_t play;
	ae_bool_t interrupt;

	ae_bool_t loop;
			
	float time;

	float work_area_begin;
	float work_area_end;
} aeMovieCompositionAnimation;
//////////////////////////////////////////////////////////////////////////
typedef struct aeMovieSubMovie
{
	const aeMovieLayerData * layer;

	aeMovieCompositionAnimation * animation;
} aeMovieSubMovie;
//////////////////////////////////////////////////////////////////////////
typedef struct aeMovieNode
{
	const aeMovieLayerData * layer;

	struct aeMovieNode * relative;
	struct aeMovieNode * track_matte;

	const aeMovieSubMovie * submovie;

	float start_time;
	float in_time;
	float out_time;

	float stretch;
	float current_time;

	ae_bool_t active;
	ae_bool_t ignore;

	uint8_t animate;

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

	const void * camera_data;
	void * element_data;
	void * track_matte_data;
} aeMovieNode;
//////////////////////////////////////////////////////////////////////////
typedef struct aeMovieComposition
{
	const aeMovieData * movie_data;
	const aeMovieCompositionData * composition_data;

	aeMovieCompositionAnimation * animation;

	uint32_t update_revision;

	ae_bool_t interpolate;

	uint32_t node_count;
	aeMovieNode * nodes;

	uint32_t submovie_count;
	aeMovieSubMovie * submovies;

	aeMovieCompositionProviders providers;
	void * provider_data;
} aeMovieComposition;

#	endif
