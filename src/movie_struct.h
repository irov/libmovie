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
	ae_movie_logerror_t logerror;
	void * instance_data;
	

	float sprite_uv[8];	
	uint16_t sprite_indices[6];	

	float bezier_warp_uv[AE_MOVIE_BEZIER_WARP_GRID_VERTEX_COUNT * 2];
	uint16_t bezier_warp_indices[AE_MOVIE_BEZIER_WARP_GRID_INDICES_COUNT];
} aeMovieInstance;
//////////////////////////////////////////////////////////////////////////
typedef struct aeMovieNode
{
	const aeMovieLayerData * layer;

	struct aeMovieNode * relative;
	struct aeMovieNode * track_matte;

	float start_time;
	float in_time;
	float out_time;

	float stretch;
	float current_time;

	ae_bool_t active;

	uint32_t animate;

	uint32_t matrix_revision;
	ae_matrix4_t matrix;

	float composition_opactity;
	float opacity;

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

	ae_bool_t play;
	ae_bool_t interrupt;

	ae_bool_t loop;

	uint32_t update_revision;
	float time;

	float work_area_begin;
	float work_area_end;

	uint32_t node_count;
	aeMovieNode * nodes;

	aeMovieCompositionProviders providers;
	void * provider_data;
} aeMovieComposition;

#	endif
