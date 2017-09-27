#ifndef MOVIE_MOVIE_NODE_H_
#define MOVIE_MOVIE_NODE_H_

#	include "movie_type.h"
#	include "movie_typedef.h"
#	include "movie_instance.h"
#	include "movie_data.h"

/**
@addtogroup data_types
@{
*/

/**
@brief Mesh used in render loop.

Can be provided:
<ul>
<li>By ae_compute_movie_mesh().</li>
<li>As a part of track matte data structure,
which is in turn created and updated inside
user-provided callbacks (see \link #ae_movie_callback_node_provider_t node provider\endlink etc.).</li>
</ul>
*/
typedef struct aeMovieRenderMesh
{
    /**
    @brief Renderable type.
    */
    ae_uint32_t layer_type;

    /**
    @brief One of AE blend modes.
    */
    aeMovieBlendMode blend_mode;

    /**
    @brief Type of the resource associated with the mesh.
    */
    aeMovieResourceTypeEnum resource_type;

    /**
    @brief Pointer to the resource data.
    */
    ae_voidptr_t resource_data;

    /**
    @brief Number of vertices.
    */
    ae_uint32_t vertexCount;

    /**
    @brief Number of indices.
    */
    ae_uint32_t indexCount;

    /**
    @brief Vertex positions.
    */
    ae_vector3_t position[AE_MOVIE_MAX_VERTICES];

    /**
    @brief Texture coordinates.
    */
    const ae_vector2_t * uv;

    /**
    @brief Triangle indices.
    */
    const ae_uint16_t * indices;

    /**
    @name Color
    @brief Usually used as a multiplier.
    @{
    */
    ae_float_t r;
    ae_float_t g;
    ae_float_t b;
    ae_float_t a;
    /// @}

    /**
    @brief Pointer to a user-provided camera structure.
    */
    ae_voidptr_t camera_data;

    /**
    @brief Pointer to a user-provided track matte structure.

    Contains track matte layer data, specifically aeMovieRenderMesh.
    */
    ae_voidptr_t track_matte_data;

    /**
    @brief Pointer to a user-provided node structure.

    Contains data for track matte, slot, socket etc.
    */
    ae_voidptr_t element_data;

    ae_voidptr_t shader_data;
} aeMovieRenderMesh;

typedef struct aeMovieNode aeMovieNode;

// data_types
/// @}



/**
@addtogroup compositions
@{
*/

typedef enum
{
    AE_MOVIE_NODE_UPDATE_BEGIN = 0,
    AE_MOVIE_NODE_UPDATE_UPDATE,
    AE_MOVIE_NODE_UPDATE_PAUSE,
    AE_MOVIE_NODE_UPDATE_RESUME,
    AE_MOVIE_NODE_UPDATE_END,
    AE_MOVIE_NODE_UPDATE_CREATE,
    AE_MOVIE_NODE_UPDATE_DESTROY,
    __AE_MOVIE_NODE_UPDATE_STATES__,
} aeMovieNodeUpdateState;

typedef struct aeMovieCameraProviderCallbackData
{
    /// @brief Camera name.
    const ae_char_t * name;

    /// @brief Field-of-view in degrees.
    ae_float_t fov;

    /// @brief Viewport width.
    ae_float_t width;

    /// @brief Viewport height.
    ae_float_t height;

    ae_vector3_t target;
    ae_vector3_t position;
    ae_quaternion_t quaternion;

} aeMovieCameraProviderCallbackData;

typedef struct aeMovieCameraDestroyCallbackData
{
    /// @brief Camera name.
    const ae_char_t * name;

    /// @brief Camera element data.
    ae_voidptr_t element;
} aeMovieCameraDestroyCallbackData;

typedef struct aeMovieCameraUpdateCallbackData
{
    /// @brief Camera element data.
    ae_voidptr_t element;

    /// @brief Camera target.
    ae_vector3_t target;

    /// @brief Camera position.
    ae_vector3_t position;

    /// @brief Camera quaternion.
    ae_quaternion_t quaternion;
} aeMovieCameraUpdateCallbackData;

typedef struct aeMovieNodeProviderCallbackData
{
    const aeMovieLayerData * layer;

    /// @brief Additional transform, e.g. for slots/sockets.
    ae_matrix4_ptr_t matrix;

    /// @brief Value from 0.0 to 1.0.
    ae_float_t opacity;

    /// @brief Pointer to track matte layer, should be used to create user track matte structure inside provider.
    const aeMovieLayerData * trackmatteLayer;
} aeMovieNodeProviderCallbackData;

typedef struct aeMovieNodeDestroyCallbackData
{
    ae_voidptr_t element;
    aeMovieLayerTypeEnum type;
} aeMovieNodeDestroyCallbackData;

typedef struct aeMovieNodeUpdateCallbackData
{
    ae_voidptr_t element;
    aeMovieLayerTypeEnum type;
    ae_bool_t loop;
    aeMovieNodeUpdateState state;
    ae_float_t offset;

    /// @brief Additional transform, e.g. for slots/sockets.
    ae_matrix4_ptr_t matrix;

    /// @brief Value from 0.0 to 1.0.
    ae_float_t opacity;
} aeMovieNodeUpdateCallbackData;

typedef struct aeMovieTrackMatteUpdateCallbackData
{
    ae_voidptr_t element;
    aeMovieLayerTypeEnum type;
    ae_bool_t loop;
    aeMovieNodeUpdateState state;
    ae_float_t offset;

    /// @brief Additional transform, e.g. for slots/sockets.
    ae_matrix4_ptr_t matrix;

    /// @brief Value from 0.0 to 1.0.
    ae_float_t opacity;

    aeMovieRenderMesh * mesh;
    ae_voidptr_t track_matte_data;
} aeMovieTrackMatteUpdateCallbackData;

typedef struct aeMovieShaderProviderCallbackData
{
    ae_string_t name;
    ae_uint32_t version;

    ae_string_t shader_vertex;
    ae_string_t shader_fragment;

    ae_uint32_t parameter_count;
    ae_string_t parameter_names[32];
} aeMovieShaderProviderCallbackData;

typedef struct aeMovieShaderPropertyUpdateCallbackData
{
    ae_voidptr_t element;

    ae_string_t name;
    ae_uint32_t type;

    ae_float_t color_r;
    ae_float_t color_g;
    ae_float_t color_b;

    ae_float_t value;
} aeMovieShaderPropertyUpdateCallbackData;

typedef struct aeMovieCompositionEventCallbackData
{
    ae_voidptr_t element;

    /// @brief Name of the composition which sent the event.
    const ae_char_t * name;

    /// @brief Additional transform, e.g. for slots/sockets.
    ae_matrix4_ptr_t matrix;

    /// @brief Value from 0.0 to 1.0.
    ae_float_t opacity;

    ae_bool_t begin;
} aeMovieCompositionEventCallbackData;

typedef enum
{
    AE_MOVIE_COMPOSITION_PLAY = 0,
    AE_MOVIE_COMPOSITION_STOP,
    AE_MOVIE_COMPOSITION_PAUSE,
    AE_MOVIE_COMPOSITION_RESUME,
    AE_MOVIE_COMPOSITION_INTERRUPT,
    AE_MOVIE_COMPOSITION_END,
    AE_MOVIE_COMPOSITION_LOOP_END,
    AE_MOVIE_SUB_COMPOSITION_PLAY,
    AE_MOVIE_SUB_COMPOSITION_STOP,
    AE_MOVIE_SUB_COMPOSITION_PAUSE,
    AE_MOVIE_SUB_COMPOSITION_RESUME,
    AE_MOVIE_SUB_COMPOSITION_INTERRUPT,
    AE_MOVIE_SUB_COMPOSITION_END,
    AE_MOVIE_SUB_COMPOSITION_LOOP_END,
    __AE_MOVIE_COMPOSITION_STATES__
} aeMovieCompositionStateFlag;

typedef struct aeMovieCompositionStateCallbackData
{
    /// @brief New composition state.
    aeMovieCompositionStateFlag state;

    /// @brief Sub-composition ???.
    const aeMovieSubComposition * subcomposition;
} aeMovieCompositionStateCallbackData;

typedef ae_voidptr_t( *ae_movie_callback_camera_provider_t )(const aeMovieCameraProviderCallbackData * _callbackData, ae_voidptr_t _data);
typedef void( *ae_movie_callback_camera_deleter_t )(const aeMovieCameraDestroyCallbackData * _callbackData, ae_voidptr_t _data);
typedef void( *ae_movie_callback_camera_update_t )(const aeMovieCameraUpdateCallbackData * _callbackData, ae_voidptr_t _data);

typedef ae_voidptr_t( *ae_movie_callback_node_provider_t )(const aeMovieNodeProviderCallbackData * _callbackData, ae_voidptr_t _data);
typedef void( *ae_movie_callback_node_deleter_t )(const aeMovieNodeDestroyCallbackData * _callbackData, ae_voidptr_t _data);
typedef void( *ae_movie_callback_node_update_t )(const aeMovieNodeUpdateCallbackData * _callbackData, ae_voidptr_t _data);

typedef ae_voidptr_t( *ae_movie_callback_track_matte_update_t )(const aeMovieTrackMatteUpdateCallbackData * _callbackData, ae_voidptr_t _data);

typedef ae_voidptr_t( *ae_movie_callback_shader_provider_t )(const aeMovieShaderProviderCallbackData * _callbackData, ae_voidptr_t _data);
typedef void( *ae_movie_callback_shader_property_update_t )(const aeMovieShaderPropertyUpdateCallbackData * _callbackData, ae_voidptr_t _data);

typedef void( *ae_movie_callback_composition_event_t )(const aeMovieCompositionEventCallbackData * _callbackData, ae_voidptr_t _data);
typedef void( *ae_movie_callback_composition_state_t )(const aeMovieCompositionStateCallbackData * _callbackData, ae_voidptr_t _data);

typedef struct aeMovieCompositionProviders
{
    ae_movie_callback_camera_provider_t camera_provider;
    ae_movie_callback_camera_deleter_t camera_deleter;
    ae_movie_callback_camera_update_t camera_update;

    ae_movie_callback_node_provider_t node_provider;
    ae_movie_callback_node_deleter_t node_deleter;
    ae_movie_callback_node_update_t node_update;

    ae_movie_callback_track_matte_update_t track_matte_update;

    ae_movie_callback_shader_provider_t shader_provider;
    ae_movie_callback_shader_property_update_t shader_property_update;

    ae_movie_callback_composition_event_t composition_event;
    ae_movie_callback_composition_state_t composition_state;

} aeMovieCompositionProviders;

void ae_initialize_movie_composition_providers( aeMovieCompositionProviders * _providers );

/**
@brief Allocate a composition in memory.
@param [in] _movieData Resource holding movie data.
@param [in] _compositionData Resource holding composition data.
@param [in] _interpolate If TRUE, composition will be interpolated.
@param [in] providers Set of callbacks which provide data used during playback.
@param [in] _data Link to the object that will hold the data providers give.
@return A composition or NULL if failed.
*/
aeMovieComposition * ae_create_movie_composition( const aeMovieData * _movieData, const aeMovieCompositionData * _compositionData, ae_bool_t _interpolate, const aeMovieCompositionProviders * providers, ae_voidptr_t _data );

/**
@brief Release a composition from memory.
@param [in] _composition Composition.
*/
void ae_delete_movie_composition( const aeMovieComposition * _composition );

/**
@brief get composition data from composition.
@param [in] _composition Composition.
*/
const aeMovieCompositionData * ae_get_movie_composition_composition_data( const aeMovieComposition * _composition );

/**
@brief Get composition anchor point.
@param [in] _composition Composition.
@param [out] _point point.
@return TRUE if successful
*/
ae_bool_t ae_get_movie_composition_anchor_point( const aeMovieComposition * _composition, ae_vector3_t _point );

/**
@param [in] _composition Composition.
@return Maximum number of render meshes you will ever get during a full ae_compute_movie_mesh() cycle.
*/
uint32_t ae_get_movie_composition_max_render_node( const aeMovieComposition * _composition );

/**
@brief Set whether a composition is looped or not.
@param [in] _composition Composition.
@param [in] _loop If TRUE, playback will be looped.
*/
void ae_set_movie_composition_loop( const aeMovieComposition * _composition, ae_bool_t _loop );

/**
@param [in] _composition Composition.
@return TRUE if composition playback is looped.
*/
ae_bool_t ae_get_movie_composition_loop( const aeMovieComposition * _composition );

/**
@param [in] _composition Composition.
@return TRUE if composition playback is interpolated.
*/
ae_bool_t ae_get_movie_composition_interpolate( const aeMovieComposition * _composition );

/**
@brief Set playback area of a composition in milliseconds.
@param [in] _composition Composition.
@param [in] _begin Begin time.
@param [in] _end End time.
@return TRUE if successful.
*/
ae_bool_t ae_set_movie_composition_work_area( const aeMovieComposition * _composition, ae_float_t _begin, ae_float_t _end );

/**
@brief Restore playback area of a composition to whole-length.
@param [in] _composition Composition.
*/
void ae_remove_movie_composition_work_area( const aeMovieComposition * _composition );

/**
@brief Start playback.
@param [in] _composition Composition.
@param [in] _time Starting time.
*/
void ae_play_movie_composition( const aeMovieComposition * _composition, ae_float_t _time );

/**
@brief Stop playback.
@param [in] _composition Composition.
*/
void ae_stop_movie_composition( const aeMovieComposition * _composition );

/**
@brief Pause playback.
@param [in] _composition Composition.
*/
void ae_pause_movie_composition( const aeMovieComposition * _composition );

/**
@brief Resume playback.
@param [in] _composition Composition.
*/
void ae_resume_movie_composition( const aeMovieComposition * _composition );

/**
@brief Play till the end of the loop area and stop, even if playback is looped.
@param [in] _composition Composition.
@param [in] _skip If TRUE, skip to the end of current animation loop.
*/
void ae_interrupt_movie_composition( const aeMovieComposition * _composition, ae_bool_t _skip );

/**
@param [in] _composition Composition.
@return TRUE if the composition is playing.
*/
ae_bool_t ae_is_play_movie_composition( const aeMovieComposition * _composition );

/**
@param [in] _composition Composition.
@return TRUE if the composition is paused.
*/
ae_bool_t ae_is_pause_movie_composition( const aeMovieComposition * _composition );

/**
@param [in] _composition Composition.
@return TRUE if the composition has been interrupted.
*/
ae_bool_t ae_is_interrupt_movie_composition( const aeMovieComposition * _composition );

/**
@brief Set composition playback to the specified position in milliseconds.
@param [in] _composition Composition.
@param [in] _timing Position.
*/
void ae_set_movie_composition_time( const aeMovieComposition * _composition, ae_float_t _timing );


const ae_char_t * ae_get_movie_composition_name( const aeMovieComposition * _composition );

/**
@brief Get composition current playback position in milliseconds.
@param [in] _composition Composition.
@return Position.
*/
float ae_get_movie_composition_time( const aeMovieComposition * _composition );

/**
@brief Get composition duration in milliseconds.
@param [in] _composition Composition.
@return Duration.
*/
float ae_get_movie_composition_duration( const aeMovieComposition * _composition );

/**
@brief Get composition loop area in milliseconds.
@param [in] _composition Composition.
@param [out] _in Begin time.
@param [out] _out End time.
*/
void ae_get_movie_composition_in_out_loop( const aeMovieComposition * _composition, ae_float_t * _in, ae_float_t * _out );

/**
@brief Update composition nodes and state. Call this from your application update() loop.
@param [in] _composition Composition.
@param [in] _timing Time offset since the last update in milliseconds.
*/
void ae_update_movie_composition( aeMovieComposition * _composition, ae_float_t _timing );

// compositions
/// @}



/**
@addtogroup slots
@{
*/

/**
@brief Set user data for a slot.
@param [in] _composition Composition.
@param [in] _slotName Slot name.
@param [in] _slotData Data pointer.
@return TRUE if successful.
*/
ae_bool_t ae_set_movie_composition_slot( const aeMovieComposition * _composition, const ae_char_t * _slotName, ae_voidptr_t _slotData );

/**
@brief Search for a slot by the given name.
@param [in] _composition Composition.
@param [in] _slotName Slot name.
@return Pointer to the slot or NULL if not found.
*/
ae_voidptr_t ae_get_movie_composition_slot( const aeMovieComposition * _composition, const ae_char_t * _slotName );

/**
@brief Check for a slot by the given name.
@param [in] _composition Composition.
@param [in] _slotName Slot name.
@return TRUE if found.
*/
ae_bool_t ae_has_movie_composition_slot( const aeMovieComposition * _composition, const ae_char_t * _slotName );

/**
@brief Remove user data from the slot.
@param [in] _composition Composition.
@param [in] _slotName Slot name.
@return Pointer to data that was assigned to this slot prior to removal.
*/
ae_voidptr_t ae_remove_movie_composition_slot( const aeMovieComposition * _composition, const ae_char_t * _slotName );

// slots
/// @}

/**
@brief get camera user data from composition.
@param [in] _composition Composition.
@return Pointer to the data referenced by the camera linked to the composition.
*/
ae_voidptr_t ae_get_movie_composition_camera_data( const aeMovieComposition * _composition );

// slots
/// @}


/**
@addtogroup sockets
@{
*/

/**
@brief Search for a socket by the given name.
@param [in] _composition Composition.
@param [in] _slotName Socket name.
@param [out] _polygon Socket shape.
@return TRUE if found.
*/
ae_bool_t ae_get_movie_composition_socket( const aeMovieComposition * _composition, const ae_char_t * _slotName, const aeMoviePolygon ** _polygon );

// sockets
/// @}



/**
@addtogroup compositions
@{
*/

/**
@brief Compute rendering data at the current playback time.
@param [in] _composition Composition.
@param [in] _iterator Index of the render mesh to update, can start from zero, will increase automatically inside.
@param [out] _vertices Pointer to render mesh.
@return TRUE if there are still meshes to compute in the next iteration, FALSE if finished.
*/
ae_bool_t ae_compute_movie_mesh( const aeMovieComposition * _composition, ae_uint32_t * _iterator, aeMovieRenderMesh * _vertices );

/**
@param [in] _composition Composition.
@return Number of meshes at the current playback time.
*/
uint32_t ae_get_movie_render_mesh_count( const aeMovieComposition * _composition );

/**
@brief Get node active time range in milliseconds.
Returns interval on which node is active, i.e. being played, rendered, etc.
@param [in] _composition Composition.
@param [in] _layerName Node name.
@param [in] _type Node type.
@param [out] _in Begin time.
@param [out] _out End time.
@return TRUE if the node is found.
*/
ae_bool_t ae_get_movie_composition_node_in_out_time( const aeMovieComposition * _composition, const ae_char_t * _layerName, aeMovieLayerTypeEnum _type, ae_float_t * _in, ae_float_t * _out );

/**
@brief Toggle layer usage.
@param [in] _composition Composition.
@param [in] _layerName Node name.
@param [in] _type Node type.
@param [in] _enable If TRUE, enable usage.
@return TRUE if the node is found.
*/
ae_bool_t ae_set_movie_composition_node_enable( const aeMovieComposition * _composition, const ae_char_t * _layerName, aeMovieLayerTypeEnum _type, ae_bool_t _enable );

/**
@brief Query whether the given node is active or not.
@param [in] _composition Composition.
@param [in] _layerName Node name.
@param [in] _type Node type.
@param [out] _enable TRUE if enabled.
@return TRUE if the node is found.
*/
ae_bool_t ae_get_movie_composition_node_enable( const aeMovieComposition * _composition, const ae_char_t * _layerName, aeMovieLayerTypeEnum _type, ae_bool_t * _enable );

// compositions
/// @}



/**
@addtogroup subcompositions
@{
*/

/**
@brief Search for a sub-composition by name.
@param [in] _composition Composition.
@param [in] _name Name of a sub-composition to search for.
@return Pointer to the sub-composition or NULL if not found.
*/
const aeMovieSubComposition * ae_get_movie_sub_composition( const aeMovieComposition * _composition, const ae_char_t * _name );

/**
@brief Get name of a sub-composition.
@param [in] _subcomposition Sub-composition.
@return Name.
*/
const char * ae_get_movie_sub_composition_name( const aeMovieSubComposition * _subcomposition );

/**
@brief Get sub-composition loop area in milliseconds.
@param [in] _subcomposition Sub-composition.
@param [out] _in Begin time.
@param [out] _out End time.
*/
void ae_get_movie_sub_composition_in_out_loop( const aeMovieSubComposition * _subcomposition, ae_float_t * _in, ae_float_t * _out );

/**
@brief Start playback.
@param [in] _composition Composition.
@param [in] _subcomposition Sub-composition.
@param [in] _time Start time in milliseconds.
@return TRUE if successful.
*/
ae_bool_t ae_play_movie_sub_composition( const aeMovieComposition * _composition, const aeMovieSubComposition * _subcomposition, ae_float_t _time );

/**
@brief Stop playback.
@param [in] _composition Composition.
@param [in] _subcomposition Sub-composition.
@return TRUE if successful.
*/
ae_bool_t ae_stop_movie_sub_composition( const aeMovieComposition * _composition, const aeMovieSubComposition * _subcomposition );

/**
@brief Pause playback.
@param [in] _composition Composition.
@param [in] _subcomposition Sub-composition.
@return TRUE if successful.
*/
ae_bool_t ae_pause_movie_sub_composition( const aeMovieComposition * _composition, const aeMovieSubComposition * _subcomposition );

/**
@brief Resume playback.
@param [in] _composition Composition.
@param [in] _subcomposition Sub-composition.
@return TRUE if successful.
*/
ae_bool_t ae_resume_movie_sub_composition( const aeMovieComposition * _composition, const aeMovieSubComposition * _subcomposition );

/**
@brief Play till the end of the loop area and stop, even if playback is looped.
@param [in] _composition Composition.
@param [in] _subcomposition Sub-composition.
@param [in] _skip If TRUE, skip to the end of current animation loop.
*/
void ae_interrupt_movie_sub_composition( const aeMovieComposition * _composition, const aeMovieSubComposition * _subcomposition, ae_bool_t _skip );

/**
@param [in] _subcomposition Sub-composition.
@return TRUE if the sub-composition is playing.
*/
ae_bool_t ae_is_play_movie_sub_composition( const aeMovieSubComposition * _subcomposition );

/**
@param [in] _subcomposition Sub-composition.
@return TRUE if the sub-composition is paused.
*/
ae_bool_t ae_is_pause_movie_sub_composition( const aeMovieSubComposition * _subcomposition );

/**
@param [in] _subcomposition Sub-composition.
@return TRUE if the sub-composition has been interrupted.
*/
ae_bool_t ae_is_interrupt_movie_sub_composition( const aeMovieSubComposition * _subcomposition );

/**
@brief Set sub-composition playback to the specified position in milliseconds.
@param [in] _composition Composition.
@param [in] _subcomposition Sub-composition.
@param [in] _timing Position.
*/
void ae_set_movie_sub_composition_time( const aeMovieComposition * _composition, const aeMovieSubComposition * _subcomposition, ae_float_t _timing );

/**
@brief Get current playback position of a sub-composition in milliseconds.
@param [in] _subcomposition Sub-composition.
@return Time.
*/
float ae_get_movie_sub_composition_time( const aeMovieSubComposition * _subcomposition );

/**
@brief Set whether a sub-composition is looped or not.
@param [in] _subcomposition Sub-composition.
@param [in] _loop If TRUE, playback will be looped.
*/
void ae_set_movie_sub_composition_loop( const aeMovieSubComposition * _subcomposition, ae_bool_t _loop );

/**
@param [in] _subcomposition Sub-composition.
@return TRUE if sub-composition playback is looped.
*/
ae_bool_t ae_get_movie_sub_composition_loop( const aeMovieSubComposition * _subcomposition );

/**
@brief Set playback area of a sub-composition in milliseconds.
@param [in] _composition Composition.
@param [in] _subcomposition Sub-composition.
@param [in] _begin Begin time.
@param [in] _end End time.
@return TRUE if successful.
*/
ae_bool_t ae_set_movie_sub_composition_work_area( const aeMovieComposition * _composition, const aeMovieSubComposition * _subcomposition, ae_float_t _begin, ae_float_t _end );

/**
@brief Restore playback area of a sub-composition to whole-length.
@param [in] _composition Composition.
@param [in] _subcomposition Sub-composition.
*/
void ae_remove_movie_sub_composition_work_area( const aeMovieComposition * _composition, const aeMovieSubComposition * _subcomposition );

// subcompositions
/// @}

#endif
