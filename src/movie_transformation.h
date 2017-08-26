/******************************************************************************
* libMOVIE Software License v1.0
*
* Copyright (c) 2016-2017, Levchenko Yuriy <irov13@mail.ru>
* All rights reserved.
*
* You are granted a perpetual, non-exclusive, non-sublicensable, and
* non-transferable license to use, install, execute, and perform the libMOVIE
* software and derivative works solely for personal or internal
* use. Without the written permission of Levchenko Yuriy, you may not (a) modify, translate,
* adapt, or develop new applications using the libMOVIE or otherwise
* create derivative works or improvements of the libMOVIE or (b) remove,
* delete, alter, or obscure any trademarks or any copyright, trademark, patent,
* or other intellectual property or proprietary rights notices on or in the
* Software, including any copy thereof. Redistributions in binary or source
* form must include this license and terms.
*
* THIS SOFTWARE IS PROVIDED BY LEVCHENKO YURIY "AS IS" AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL LEVCHENKO YURIY BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES, BUSINESS INTERRUPTION,
* OR LOSS OF USE, DATA, OR PROFITS) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#	ifndef MOVIE_TRANSFORMATION_H_
#	define MOVIE_TRANSFORMATION_H_

#	include "movie/movie_instance.h"
#	include "movie/movie_type.h"

typedef enum aeMoviePropertyImmutableEnum
{
    AE_MOVIE_IMMUTABLE_ANCHOR_POINT_X = 0x00000001,
    AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Y = 0x00000002,
    AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Z = 0x00000004,
    AE_MOVIE_IMMUTABLE_POSITION_X = 0x00000010,
    AE_MOVIE_IMMUTABLE_POSITION_Y = 0x00000020,
    AE_MOVIE_IMMUTABLE_POSITION_Z = 0x00000040,
    AE_MOVIE_IMMUTABLE_SCALE_X = 0x00000100,
    AE_MOVIE_IMMUTABLE_SCALE_Y = 0x00000200,
    AE_MOVIE_IMMUTABLE_SCALE_Z = 0x00000400,
    AE_MOVIE_IMMUTABLE_QUATERNION_X = 0x00001000,
    AE_MOVIE_IMMUTABLE_QUATERNION_Y = 0x00002000,
    AE_MOVIE_IMMUTABLE_QUATERNION_Z = 0x00004000,
    AE_MOVIE_IMMUTABLE_QUATERNION_W = 0x00008000,
    AE_MOVIE_IMMUTABLE_OPACITY = 0x00010000,
    AE_MOVIE_IMMUTABLE_TARGET_X = 0x00100000,
    AE_MOVIE_IMMUTABLE_TARGET_Y = 0x00200000,
    AE_MOVIE_IMMUTABLE_TARGET_Z = 0x00400000,
    __AE_MOVIE_IMMUTABLE_NONE__ = 0,
    __AE_MOVIE_IMMUTABLE_TWO_D_ALL__ = 0
    | AE_MOVIE_IMMUTABLE_ANCHOR_POINT_X | AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Y
    | AE_MOVIE_IMMUTABLE_POSITION_X | AE_MOVIE_IMMUTABLE_POSITION_Y
    | AE_MOVIE_IMMUTABLE_SCALE_X | AE_MOVIE_IMMUTABLE_SCALE_Y
    | AE_MOVIE_IMMUTABLE_QUATERNION_Z | AE_MOVIE_IMMUTABLE_QUATERNION_W,
    __AE_MOVIE_IMMUTABLE_THREE_D_ALL__ = 0
    | AE_MOVIE_IMMUTABLE_ANCHOR_POINT_X | AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Y | AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Z
    | AE_MOVIE_IMMUTABLE_POSITION_X | AE_MOVIE_IMMUTABLE_POSITION_Y | AE_MOVIE_IMMUTABLE_POSITION_Z
    | AE_MOVIE_IMMUTABLE_SCALE_X | AE_MOVIE_IMMUTABLE_SCALE_Y | AE_MOVIE_IMMUTABLE_SCALE_Z
    | AE_MOVIE_IMMUTABLE_QUATERNION_X | AE_MOVIE_IMMUTABLE_QUATERNION_Y | AE_MOVIE_IMMUTABLE_QUATERNION_Z | AE_MOVIE_IMMUTABLE_QUATERNION_W,
    __AE_MOVIE_IMMUTABLE_CAMERA_ALL__ = 0
    | AE_MOVIE_IMMUTABLE_TARGET_X | AE_MOVIE_IMMUTABLE_TARGET_Y | AE_MOVIE_IMMUTABLE_TARGET_Z
    | AE_MOVIE_IMMUTABLE_POSITION_X | AE_MOVIE_IMMUTABLE_POSITION_Y | AE_MOVIE_IMMUTABLE_POSITION_Z
    | AE_MOVIE_IMMUTABLE_QUATERNION_X | AE_MOVIE_IMMUTABLE_QUATERNION_Y | AE_MOVIE_IMMUTABLE_QUATERNION_Z | AE_MOVIE_IMMUTABLE_QUATERNION_W,
    __AE_MOVIE_IMMUTABLE_END__
} aeMoviePropertyImmutableEnum;

typedef struct aeMovieLayerTransformation2DImuttable
{
    ae_float_t anchor_point_x;
    ae_float_t anchor_point_y;
    ae_float_t position_x;
    ae_float_t position_y;
    ae_float_t scale_x;
    ae_float_t scale_y;
    ae_float_t quaternion_z;
    ae_float_t quaternion_w;

}aeMovieLayerTransformation2DImuttable;

typedef struct aeMovieLayerTransformation3DImuttable
{
    ae_float_t anchor_point_x;
    ae_float_t anchor_point_y;
    ae_float_t anchor_point_z;
    ae_float_t position_x;
    ae_float_t position_y;
    ae_float_t position_z;
    ae_float_t scale_x;
    ae_float_t scale_y;
    ae_float_t scale_z;
    ae_float_t quaternion_x;
    ae_float_t quaternion_y;
    ae_float_t quaternion_z;
    ae_float_t quaternion_w;

}aeMovieLayerTransformation3DImuttable;

typedef struct aeMovieLayerTransformation2DTimeline
{
    ae_constvoidptr_t anchor_point_x;
    ae_constvoidptr_t anchor_point_y;
    ae_constvoidptr_t position_x;
    ae_constvoidptr_t position_y;
    ae_constvoidptr_t quaternion_z;
    ae_constvoidptr_t quaternion_w;
    ae_constvoidptr_t scale_x;
    ae_constvoidptr_t scale_y;

}aeMovieLayerTransformation2DTimeline;

typedef struct aeMovieLayerTransformation3DTimeline
{
    ae_constvoidptr_t anchor_point_x;
    ae_constvoidptr_t anchor_point_y;
    ae_constvoidptr_t anchor_point_z;
    ae_constvoidptr_t position_x;
    ae_constvoidptr_t position_y;
    ae_constvoidptr_t position_z;
    ae_constvoidptr_t quaternion_x;
    ae_constvoidptr_t quaternion_y;
    ae_constvoidptr_t quaternion_z;
    ae_constvoidptr_t quaternion_w;
    ae_constvoidptr_t scale_x;
    ae_constvoidptr_t scale_y;
    ae_constvoidptr_t scale_z;

}aeMovieLayerTransformation3DTimeline;

typedef struct aeMovieLayerTransformation
{
    ae_uint32_t immutable_property_mask;

    ae_float_t immutable_opacity;
    ae_constvoidptr_t timeline_opacity;

}aeMovieLayerTransformation;

typedef struct aeMovieLayerTransformation2D
{
    ae_uint32_t immutable_property_mask;

    ae_float_t immutable_opacity;
    ae_constvoidptr_t timeline_opacity;

    aeMovieLayerTransformation2DImuttable immutable;
    aeMovieLayerTransformation2DTimeline * timeline;

} aeMovieLayerTransformation2D;

typedef struct aeMovieLayerTransformation3D
{
    ae_uint32_t immutable_property_mask;

    ae_float_t immutable_opacity;
    ae_constvoidptr_t timeline_opacity;

    aeMovieLayerTransformation3DImuttable immutable;
    aeMovieLayerTransformation3DTimeline * timeline;

} aeMovieLayerTransformation3D;


ae_result_t ae_movie_load_layer_transformation( aeMovieStream * _stream, aeMovieLayerTransformation * _transformation, ae_bool_t _threeD );
ae_result_t ae_movie_load_camera_transformation( aeMovieStream * _stream, aeMovieCompositionCamera * _camera );
void ae_movie_delete_layer_transformation( const aeMovieInstance * _instance, const aeMovieLayerTransformation * _transformation, ae_bool_t _threeD );
ae_float_t ae_movie_make_layer_opacity( const aeMovieLayerTransformation * _transformation, ae_uint32_t _index, ae_bool_t _interpolate, ae_float_t _t );
void ae_movie_make_layer_transformation( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_bool_t _threeD, ae_uint32_t _index, ae_bool_t _interpolate, ae_float_t _t );
void ae_movie_make_camera_transformation( ae_vector3_t _target, ae_vector3_t _position, ae_quaternion_t _quaternion, const aeMovieCompositionCamera * _camera, ae_uint32_t _index, ae_bool_t _interpolate, ae_float_t _t );

#	endif