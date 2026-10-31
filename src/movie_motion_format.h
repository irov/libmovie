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

#ifndef MOVIE_MOTION_FORMAT_H_
#define MOVIE_MOTION_FORMAT_H_
#include "movie/movie_type.h"
//////////////////////////////////////////////////////////////////////////
enum aeMovieDataExtension
{
    AE_MOVIE_DATA_EXTENSION_MOTION = 1
};
//////////////////////////////////////////////////////////////////////////
enum aeMotionProperty
{
    AE_MOTION_X,
    AE_MOTION_Y,
    AE_MOTION_ANCHOR_X,
    AE_MOTION_ANCHOR_Y,
    AE_MOTION_SCALE_X,
    AE_MOTION_SCALE_Y,
    AE_MOTION_ROTATION,
    AE_MOTION_SKEW,
    AE_MOTION_RED,
    AE_MOTION_GREEN,
    AE_MOTION_BLUE,
    AE_MOTION_OPACITY,
    AE_MOTION_VOLUME,
    AE_MOTION_PATH,
    AE_MOTION_CUSTOM
};
//////////////////////////////////////////////////////////////////////////
enum aeMotionOpcode
{
    AE_MOTION_CONST,
    AE_MOTION_TIME,
    AE_MOTION_VALUE,
    AE_MOTION_PARAM,
    AE_MOTION_REF,
    AE_MOTION_ADD,
    AE_MOTION_SUB,
    AE_MOTION_MUL,
    AE_MOTION_DIV,
    AE_MOTION_MOD,
    AE_MOTION_NEG,
    AE_MOTION_SIN,
    AE_MOTION_COS,
    AE_MOTION_ABS,
    AE_MOTION_EXP,
    AE_MOTION_SQRT,
    AE_MOTION_MIN,
    AE_MOTION_MAX,
    AE_MOTION_CLAMP,
    AE_MOTION_LERP,
    AE_MOTION_SMOOTH,
    AE_MOTION_NOISE,
    AE_MOTION_WIGGLE,
    AE_MOTION_SPRING,
    AE_MOTION_POW,
    AE_MOTION_LT,
    AE_MOTION_LE,
    AE_MOTION_GT,
    AE_MOTION_GE,
    AE_MOTION_EQ,
    AE_MOTION_NE,
    AE_MOTION_SELECT,
    AE_MOTION_BASE_REF
};
//////////////////////////////////////////////////////////////////////////
typedef struct aeMotionKey
{
    ae_float_t time;
    ae_float_t value;
    ae_float_t in_time;
    ae_float_t in_value;
    ae_float_t out_time;
    ae_float_t out_value;
    ae_uint32_t interpolation;
} aeMotionKey;
//////////////////////////////////////////////////////////////////////////
typedef struct aeMotionInstruction
{
    ae_uint32_t opcode;
    ae_uint32_t index;
    ae_float_t value;
} aeMotionInstruction;

ae_float_t ae_movie_motion_sample_keys( const aeMotionKey * _keys, ae_uint32_t _count, ae_float_t _time );
#endif
