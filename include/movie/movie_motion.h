/******************************************************************************
* libMOVIE Software License v1.0
*
* Copyright (c) 2016-2026, Yuriy Levchenko <irov13@mail.ru>
* All rights reserved.
*
* You are granted a perpetual, non-exclusive, non-sublicensable, and
* non-transferable license to use, install, execute, and perform the libMOVIE
* software and derivative works solely for personal or internal
* use. Without the written permission of Yuriy Levchenko, you may not (a) modify, translate,
* adapt, or develop new applications using the libMOVIE or otherwise
* create derivative works or improvements of the libMOVIE or (b) remove,
* delete, alter, or obscure any trademarks or any copyright, trademark, patent,
* or other intellectual property or proprietary rights notices on or in the
* Software, including any copy thereof. Redistributions in binary or source
* form must include this license and terms.
*
* THIS SOFTWARE IS PROVIDED BY YURIY LEVCHENKO "AS IS" AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL YURIY LEVCHENKO BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES, BUSINESS INTERRUPTION,
* OR LOSS OF USE, DATA, OR PROFITS) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#ifndef MOVIE_MOTION_H_
#define MOVIE_MOTION_H_

#include "movie_type.h"
//////////////////////////////////////////////////////////////////////////
typedef enum aeMovieMotionParameter
{
    AE_MOVIE_MOTION_PARAMETER_SPEED,
    AE_MOVIE_MOTION_PARAMETER_INTENSITY,
    AE_MOVIE_MOTION_PARAMETER_AMPLITUDE,
    AE_MOVIE_MOTION_PARAMETER_FREQUENCY,
    AE_MOVIE_MOTION_PARAMETER_PHASE,
    AE_MOVIE_MOTION_PARAMETER_PROGRESS,
    AE_MOVIE_MOTION_PARAMETER_BLEND,
    AE_MOVIE_MOTION_PARAMETER_DIRECTION_X,
    AE_MOVIE_MOTION_PARAMETER_DIRECTION_Y,
    AE_MOVIE_MOTION_PARAMETER_TARGET_X,
    AE_MOVIE_MOTION_PARAMETER_TARGET_Y,
    AE_MOVIE_MOTION_PARAMETER_DAMPING,
    AE_MOVIE_MOTION_PARAMETER_SEED,
    AE_MOVIE_MOTION_PARAMETER_COUNT
} aeMovieMotionParameter;
//////////////////////////////////////////////////////////////////////////
typedef struct aeMovieMarkerCallbackData
{
    const ae_char_t * name;
    ae_time_t time;
    ae_uint32_t iteration;
} aeMovieMarkerCallbackData;

typedef ae_void_t ( *ae_movie_marker_callback_t )( const aeMovieMarkerCallbackData * _data, ae_userdata_t _userdata );

ae_bool_t ae_is_movie_motion_data( const aeMovieData * _data );
ae_uint32_t ae_get_movie_motion_channel_count( const aeMovieData * _data );
const ae_char_t * ae_get_movie_motion_channel_name( const aeMovieData * _data, ae_uint32_t _index );
ae_bool_t ae_evaluate_movie_motion_channel( const aeMovieComposition * _composition, const ae_char_t * _name, ae_time_t _time, ae_float_t * _value );
const ae_char_t * ae_get_movie_motion_parameter_name( aeMovieMotionParameter _parameter );
ae_float_t ae_get_movie_motion_parameter_default( aeMovieMotionParameter _parameter );
ae_bool_t ae_set_movie_motion_parameter( const aeMovieComposition * _composition, aeMovieMotionParameter _parameter, ae_float_t _value );
ae_bool_t ae_get_movie_motion_parameter( const aeMovieComposition * _composition, aeMovieMotionParameter _parameter, ae_float_t * _value );
ae_uint32_t ae_get_movie_motion_register_count( const aeMovieData * _data );
const ae_char_t * ae_get_movie_motion_register_name( const aeMovieData * _data, ae_uint32_t _index );
ae_bool_t ae_find_movie_motion_register( const aeMovieData * _data, const ae_char_t * _name, ae_uint32_t * _index );
ae_bool_t ae_set_movie_motion_register( const aeMovieComposition * _composition, ae_uint32_t _index, ae_float_t _value );
ae_bool_t ae_get_movie_motion_register( const aeMovieComposition * _composition, ae_uint32_t _index, ae_float_t * _value );
ae_void_t ae_set_movie_marker_callback( const aeMovieComposition * _composition, ae_movie_marker_callback_t _callback, ae_userdata_t _userdata );

ae_bool_t ae_finish_movie_motion( const aeMovieComposition * _composition );

#endif
