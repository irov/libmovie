#ifndef MOVIE_MOVIE_DATA_H_
#define MOVIE_MOVIE_DATA_H_

#	include "movie_type.h"
#	include "movie_typedef.h"
#	include "movie_instance.h"

/**
@addtogroup data_types
@{
*/

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

// data_types
/// @}

/**
@addtogroup data
@{
*/

/**
@brief Allocate a data structure to load movie file into.
@param [in] _instance Instance.
@return Pointer to the created structure.
*/
aeMovieData * ae_create_movie_data( const aeMovieInstance * _instance );

/**
@brief Release data.
@param [in] _movieData Data.
*/
void ae_delete_movie_data( const aeMovieData * _movieData );

/**
@brief Callback to allocate a new resource given by the descriptor.
@param [in] _resource Description of the resource to load.
@param [in] _data Object which will hold the resource reference after loading.
@return Reference to the created resource.
*/
typedef void * (*ae_movie_data_resource_provider_t)(const aeMovieResource * _resource, void * _data);

/**
@brief Create a stream to load the data from the given data pointer.
@param [in] _instance Instance.
@param [in] _read,_copy User pointers to utility functions.
@param [in] _data Object to use in above callbacks to read data from.
@return Pointer to the stream.
*/
aeMovieStream * ae_create_movie_stream( const aeMovieInstance * _instance, ae_movie_stream_memory_read_t _read, ae_movie_stream_memory_copy_t _copy, void * _data );

/**
@brief Release stream.
@param [in] _stream Stream.
*/
void ae_delete_movie_stream( aeMovieStream * _stream );

/**
@brief Fills movie data structure and loads resources through provider.
@param [in] _movieData Data structure to fill.
@param [in] _stream Object to load from.
@param [in] _provider See \link #ae_movie_data_resource_provider_t resource provider\endlink.
@param [in] _data Object which will hold the resource reference after loading.
@return TRUE if successful.
*/
ae_result_t ae_load_movie_data( aeMovieData * _movieData, aeMovieStream * _stream, ae_movie_data_resource_provider_t _provider, void * _data );

/**
@brief Search for composition data by the given name.
@param [in] _movieData Data.
@param [in] _name Composition name.
@return Pointer to the data or NULL if not found.
*/
const aeMovieCompositionData * ae_get_movie_composition_data( const aeMovieData * _movieData, const ae_char_t * _name );

/**
@param [in] _layer Layer.
@return Layer name.
*/
const char * ae_get_movie_layer_data_name( const aeMovieLayerData * _layer );

/**
@param [in] _layer Layer.
@return Layer type.
*/
aeMovieLayerTypeEnum ae_get_movie_layer_data_type( const aeMovieLayerData * _layer );

/**
@param [in] _layer Layer.
@return TRUE if the layer is a track matte layer.
*/
ae_bool_t ae_is_movie_layer_data_track_mate( const aeMovieLayerData * _layer );

/**
@param [in] _layer Layer.
@return TRUE if the layer data is three-dimensional.
*/
ae_bool_t ae_is_movie_layer_data_threeD( const aeMovieLayerData * _layer );

/**
@param [in] _layer Layer.
@return Pointer to the data referenced by the resource linked to the layer.
*/
void * ae_get_movie_layer_data_resource_data( const aeMovieLayerData * _layer );

/**
@param [in] _layer Layer.
@return Layer blend mode.
*/
aeMovieBlendMode ae_get_movie_layer_data_blend_mode( const aeMovieLayerData * _layer );

/**
@param [in] _compositionData Composition data.
@return Composition name.
*/
const char * ae_get_movie_composition_data_name( const aeMovieCompositionData * _compositionData );

/**
@param [in] _compositionData Composition data.
@return Composition duration in milliseconds.
*/
float ae_get_movie_composition_data_duration( const aeMovieCompositionData * _compositionData );

/**
@param [in] _compositionData Composition data.
@return Single frame duration in milliseconds.
*/
float ae_get_movie_composition_data_frame_duration( const aeMovieCompositionData * _compositionData );

/**
@param [in] _compositionData Composition data.
@return Total number of frames in the composition.
*/
uint32_t ae_get_movie_composition_data_frame_count( const aeMovieCompositionData * _compositionData );

/**
@brief Get composition loop range in milliseconds.
@param [in] _compositionData Composition data.
@param [out] _in Begin time.
@param [out] _out End time.
*/
void ae_get_movie_composition_data_loop_segment( const aeMovieCompositionData * _compositionData, float * _in, float * _out );

/**
@param [in] _compositionData Composition data.
@return ???
*/
ae_bool_t ae_get_movie_composition_data_master( const aeMovieCompositionData * _compositionData );

/**
@param [in] _movieData Data.
@return Total number of compositions in the movie.
*/
uint32_t ae_get_movie_composition_data_count( const aeMovieData * _movieData );

/**
@param [in] _movieData Data.
@param [in] _index Composition index.
@return Composition data under the given index.
*/
const aeMovieCompositionData * ae_get_movie_composition_data_by_index( const aeMovieData * _movieData, uint32_t _index );

/**
@param [in] _compositionData Composition data.
@return Total number of events.
*/
uint32_t ae_get_composition_data_event_count( const aeMovieCompositionData * _compositionData );

/**
@param [in] _compositionData Composition data.
@param [in] _index Event index.
@return Name of the event under the given index.
*/
const ae_char_t * ae_get_composition_data_event_name( const aeMovieCompositionData * _compositionData, uint32_t _index );

// data
/// @}

#endif