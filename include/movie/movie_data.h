#ifndef MOVIE_MOVIE_DATA_H_
#define MOVIE_MOVIE_DATA_H_

#	include "movie_type.h"
#	include "movie_typedef.h"

#	include "movie_instance.h"
#	include "movie_resource.h"

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

typedef enum aeMovieCompositionFlag
{
	AE_MOVIE_COMPOSITION_LOOP_SEGMENT = 0x00000001,
	AE_MOVIE_COMPOSITION_ANCHOR_POINT = 0x00000002,
	AE_MOVIE_COMPOSITION_OFFSET_POINT = 0x00000004,
	AE_MOVIE_COMPOSITION_BOUNDS = 0x00000008,
	__AE_MOVIE_COMPOSITION_FLAGS__
} aeMovieCompositionFlag;

aeMovieData * ae_create_movie_data( const aeMovieInstance * _instance );
void ae_delete_movie_data( const aeMovieData * _movieData );

typedef void * (*ae_movie_data_resource_provider_t)(const aeMovieResource * _resource, void * _data);

aeMovieStream * ae_create_movie_stream( const aeMovieInstance * _instance, ae_movie_stream_memory_read_t _read, ae_movie_stream_memory_copy_t _copy, void * _data );
void ae_delete_movie_stream( aeMovieStream * _stream );

ae_result_t ae_load_movie_data( aeMovieData * _movieData, aeMovieStream * _stream, ae_movie_data_resource_provider_t _provider, void * _data );

typedef ae_bool_t( *ae_movie_data_tream_image_resource_t )(const aeMovieResourceImage * _imageResource, float * base_width, float * base_height, float * trim_width, float * trim_height, float * offset_x, float * offset_y, void * _data);

void ae_trim_image_resources( aeMovieData * _movieData, ae_movie_data_tream_image_resource_t _provider, void * _data );

const aeMovieCompositionData * ae_get_movie_composition_data( const aeMovieData * _movieData, const ae_char_t * _name );

const char * ae_get_movie_composition_data_name( const aeMovieCompositionData * _compositionData );
float ae_get_movie_composition_data_duration( const aeMovieCompositionData * _compositionData );
float ae_get_movie_composition_data_frame_duration( const aeMovieCompositionData * _compositionData );
uint32_t ae_get_movie_composition_data_frame_count( const aeMovieCompositionData * _compositionData );
void ae_get_movie_composition_data_loop_segment( const aeMovieCompositionData * _compositionData, float * _in, float * _out );

ae_bool_t ae_get_movie_composition_data_master( const aeMovieCompositionData * _compositionData );

uint32_t ae_get_movie_composition_data_count( const aeMovieData * _movieData );
const aeMovieCompositionData * ae_get_movie_composition_data_by_index( const aeMovieData * _movieData, uint32_t _index );

uint32_t ae_get_composition_data_event_count( const aeMovieCompositionData * _compositionData );
const ae_char_t * ae_get_composition_data_event_name( const aeMovieCompositionData * _compositionData, uint32_t _index );

#endif