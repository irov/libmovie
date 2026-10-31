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

#ifndef MOVIE_MOTION_INTERNAL_H_
#define MOVIE_MOTION_INTERNAL_H_

#include <stdint.h>

#include "movie_struct.h"
#include "movie/movie_motion.h"

#include "movie_motion_format.h"
//////////////////////////////////////////////////////////////////////////
typedef struct aeMotionTrack
{
    ae_char_t * name;
    ae_uint32_t composition;
    ae_uint32_t layer;
    ae_uint32_t property;
    ae_uint32_t extrapolation;
    ae_float_t value;
    ae_uint32_t key_count;
    ae_uint32_t instruction_count;
    ae_uint32_t depth;
    aeMotionKey * keys;
    aeMotionInstruction * instructions;
} aeMotionTrack;
//////////////////////////////////////////////////////////////////////////
typedef struct aeMotionParameter
{
    ae_char_t * name;
    ae_float_t value;
    ae_uint32_t slot;
} aeMotionParameter;
//////////////////////////////////////////////////////////////////////////
typedef struct aeMotionMarker
{
    ae_char_t * name;
    ae_uint32_t composition;
    ae_float_t time;
} aeMotionMarker;
//////////////////////////////////////////////////////////////////////////
typedef struct aeMotionPath
{
    ae_uint32_t composition;
    ae_uint32_t layer;
    ae_uint32_t channel;
    ae_uint32_t count;
    ae_vector2_t * points;
} aeMotionPath;
//////////////////////////////////////////////////////////////////////////
typedef struct aeMovieMotionData
{
    ae_uint32_t parameter_count;
    ae_uint32_t register_count;
    ae_uint32_t track_count;
    ae_uint32_t marker_count;
    ae_uint32_t path_count;
    aeMotionParameter * parameters;
    aeMotionTrack * tracks;
    aeMotionMarker * markers;
    aeMotionPath * paths;
} aeMovieMotionData;
//////////////////////////////////////////////////////////////////////////
typedef struct aeMovieMotionState
{
    ae_float_t * parameters;
    ae_float_t * values;
    ae_float_t * times;
    uint64_t * revisions;
    uint64_t parameter_revision;
    uint64_t pose_revision;
    ae_movie_marker_callback_t callback;
    ae_userdata_t userdata;
    ae_uint32_t iteration;
    ae_bool_t started;
    ae_bool_t finished;
} aeMovieMotionState;

ae_result_t ae_movie_motion_load( aeMovieData * _data, aeMovieStream * _stream );
ae_void_t ae_movie_motion_delete( const aeMovieData * _data );
ae_bool_t ae_movie_motion_create_state( aeMovieComposition * _composition );
ae_void_t ae_movie_motion_delete_state( const aeMovieComposition * _composition );
ae_void_t ae_movie_motion_update_node( aeMovieNode * _node, const aeMovieComposition * _composition, ae_uint32_t _frame, ae_float_t _fraction );
ae_void_t ae_movie_motion_dispatch( const aeMovieComposition * _composition, ae_time_t _delta );
ae_void_t ae_movie_motion_restart( const aeMovieComposition * _composition );
ae_float_t ae_movie_motion_channel_value( const aeMovieComposition * _composition, ae_uint32_t _index, ae_float_t _time );

#endif
