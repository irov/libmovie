/******************************************************************************
* libMOVIE Software License v1.0
*
* Copyright (c) 2016-2018, Yuriy Levchenko <irov13@mail.ru>
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

#include "movie_transformation.h"

#include "movie_stream.h"
#include "movie_math.h"

//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_constvoidptr_t __load_movie_layer_transformation_timeline( aeMovieStream * _stream, const ae_char_t * _doc )
{
    AE_UNUSED( _doc );

    ae_uint32_t zp_data_size ;
    AE_READ( _stream, zp_data_size );

    ae_uint32_t hashmask_iterator = AE_READ8( _stream );

    ae_voidptr_t timeline = AE_NEWV( _stream->instance, zp_data_size, _doc );
    
    AE_MOVIE_PANIC_MEMORY( timeline, AE_NULL );

    AE_READV( _stream, timeline, (ae_size_t)zp_data_size );

    const ae_uint32_t * hashmask = _stream->instance->hashmask;

    ae_uint32_t * it_timeline = (ae_uint32_t *)timeline;
    ae_uint32_t * it_timeline_end = (ae_uint32_t *)timeline + zp_data_size / 4U;
    for( ; it_timeline != it_timeline_end; ++it_timeline )
    {
        ae_uint32_t hashmask_index = (hashmask_iterator++) % 5U;

        ae_uint32_t hash = hashmask[hashmask_index];

        *it_timeline ^= hash;
    }

    return timeline;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_float_t __get_movie_layer_transformation_property( ae_constvoidptr_t _property, ae_uint32_t _index )
{
    ae_uint32_t property_block_offset[4] = { 1U, 3U, 0U, 0U };

    ae_uint32_t property_index = 0U;

    const ae_uint32_t * property_ae_uint32_t = (const ae_uint32_t *)_property;
    
    ae_uint32_t zp_block_type_count_data = *(property_ae_uint32_t++);

    ae_uint32_t zp_block_type = zp_block_type_count_data >> 24U;
    ae_uint32_t zp_block_count = zp_block_type_count_data & 0x00FFFFFF;

    while( property_index + zp_block_count <= _index )
    {
        property_block_offset[3] = zp_block_count;

        property_ae_uint32_t += property_block_offset[zp_block_type];

        property_index += zp_block_count;

        ae_uint32_t zp_block_type_count_data_next = *(property_ae_uint32_t++);

        zp_block_type = zp_block_type_count_data_next >> 24U;
        zp_block_count = zp_block_type_count_data_next & 0x00FFFFFF;
    }

    const ae_float_t * property_ae_float_t = (const ae_float_t *)(ae_constvoidptr_t)(property_ae_uint32_t);

    switch( zp_block_type )
    {
    case 0:
        {
            ae_float_t block_value = property_ae_float_t[0];

            return block_value;
        }break;
    case 1:
        {
            ae_float_t block_inv = property_ae_float_t[0];
            ae_float_t block_begin = property_ae_float_t[1];
            ae_float_t block_end = property_ae_float_t[2];

            ae_float_t block_add = (block_end - block_begin) * block_inv;

            ae_uint32_t block_index = _index - property_index;
            ae_float_t block_index_f = (ae_float_t)block_index;

            ae_float_t block_value = block_begin + block_add * block_index_f;

            return block_value;
        }break;
    case 3:
        {
            ae_uint32_t block_index = _index - property_index;

            ae_float_t block_value = property_ae_float_t[block_index];

            return block_value;
        }break;
    default:
        {
            //Error
        }break;
    }

    __movie_break_point();

    return 0.f;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_float_t __get_movie_layer_transformation_property_interpolate( ae_constvoidptr_t _property, ae_uint32_t _index, ae_float_t _t )
{
    ae_float_t data_0 = __get_movie_layer_transformation_property( _property, _index + 0 );
    ae_float_t data_1 = __get_movie_layer_transformation_property( _property, _index + 1 );

    ae_float_t data = ae_linerp_f1( data_0, data_1, _t );

    return data;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_void_t __make_movie_layer_transformation2d_immutable( ae_matrix4_t _out, const aeMovieLayerTransformation2D * _transformation )
{
    ae_vector2_t anchor_point;
    anchor_point[0] = _transformation->immutable.anchor_point_x;
    anchor_point[1] = _transformation->immutable.anchor_point_y;

    ae_vector2_t position;
    position[0] = _transformation->immutable.position_x;
    position[1] = _transformation->immutable.position_y;

    ae_vector2_t scale;
    scale[0] = _transformation->immutable.scale_x;
    scale[1] = _transformation->immutable.scale_y;
    
    ae_quaternionzw_t quaternionzw;
    quaternionzw[0] = _transformation->immutable.quaternion_z;
    quaternionzw[1] = _transformation->immutable.quaternion_w;

    ae_skew_t skew;
    skew[0] = _transformation->immutable.skew;
    skew[1] = _transformation->immutable.skew_quaternion_z;
    skew[2] = _transformation->immutable.skew_quaternion_w;
        
    ae_movie_make_transformation2d_m4( _out, position, anchor_point, scale, quaternionzw, skew );
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_void_t __make_movie_layer_transformation3d_immutable( ae_matrix4_t _out, const aeMovieLayerTransformation3D * _transformation )
{
    ae_vector3_t anchor_point;
    anchor_point[0] = _transformation->immutable.anchor_point_x;
    anchor_point[1] = _transformation->immutable.anchor_point_y;
    anchor_point[2] = _transformation->immutable.anchor_point_z;

    ae_vector3_t position;
    position[0] = _transformation->immutable.position_x;
    position[1] = _transformation->immutable.position_y;
    position[2] = _transformation->immutable.position_z;

    ae_vector3_t scale;
    scale[0] = _transformation->immutable.scale_x;
    scale[1] = _transformation->immutable.scale_y;
    scale[2] = _transformation->immutable.scale_z;

    ae_quaternion_t quaternion;
    quaternion[0] = _transformation->immutable.quaternion_x;
    quaternion[1] = _transformation->immutable.quaternion_y;
    quaternion[2] = _transformation->immutable.quaternion_z;
    quaternion[3] = _transformation->immutable.quaternion_w;

    ae_skew_t skew;
    skew[0] = _transformation->immutable.skew;
    skew[1] = _transformation->immutable.skew_quaternion_z;
    skew[2] = _transformation->immutable.skew_quaternion_w;
    
    ae_movie_make_transformation3d_m4( _out, position, anchor_point, scale, quaternion, skew );
}
//////////////////////////////////////////////////////////////////////////
#define AE_STREAM_PROPERTY(Mask, Name)\
	if( _mask & Mask )\
	{\
		AE_READ( _stream, _transformation->immutable.Name );\
		if( _transformation->timeline != AE_NULL ) {_transformation->timeline->Name = AE_NULL;}\
	}\
	else\
	{\
		_transformation->immutable.Name = 0.f;\
		_transformation->timeline->Name = __load_movie_layer_transformation_timeline(_stream, #Name);\
        AE_RESULT_PANIC_MEMORY(_transformation->timeline->Name);\
	}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_result_t __load_movie_layer_transformation2d( aeMovieStream * _stream, ae_uint32_t _mask, aeMovieLayerTransformation2D * _transformation )
{
    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_X, anchor_point_x );
    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Y, anchor_point_y );

    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_X, position_x );
    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_Y, position_y );

    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_X, scale_x );
    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_Y, scale_y );

    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_Z, quaternion_z );
    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_W, quaternion_w );

    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SKEW, skew );
    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SKEW_QUATERNION_Z, skew_quaternion_z );
    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SKEW_QUATERNION_W, skew_quaternion_w );

    if( (_transformation->immutable_property_mask & AE_MOVIE_IMMUTABLE_SUPER_TWO_D_ALL) == AE_MOVIE_IMMUTABLE_SUPER_TWO_D_ALL )
    {
        ae_matrix4_t * immutable_matrix = AE_NEW( _stream->instance, ae_matrix4_t );
        
        AE_MOVIE_PANIC_MEMORY( immutable_matrix, AE_RESULT_INVALID_MEMORY );

        __make_movie_layer_transformation2d_immutable( *immutable_matrix, _transformation );

        _transformation->immutable_matrix = immutable_matrix;
    }
    else
    {
        _transformation->immutable_matrix = AE_NULL;
    }

    return AE_RESULT_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_result_t __load_movie_layer_transformation3d( aeMovieStream * _stream, ae_uint32_t _mask, aeMovieLayerTransformation3D * _transformation )
{
    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_X, anchor_point_x );
    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Y, anchor_point_y );
    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Z, anchor_point_z );

    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_X, position_x );
    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_Y, position_y );
    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_Z, position_z );

    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_X, scale_x );
    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_Y, scale_y );
    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_Z, scale_z );

    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_X, quaternion_x );
    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_Y, quaternion_y );
    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_Z, quaternion_z );
    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_W, quaternion_w );

    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SKEW, skew );
    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SKEW_QUATERNION_Z, skew_quaternion_z );
    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SKEW_QUATERNION_W, skew_quaternion_w );

    if( (_transformation->immutable_property_mask & AE_MOVIE_IMMUTABLE_SUPER_THREE_D_ALL) == AE_MOVIE_IMMUTABLE_SUPER_THREE_D_ALL )
    {
        ae_matrix4_t * immutable_matrix = AE_NEW( _stream->instance, ae_matrix4_t );

        AE_MOVIE_PANIC_MEMORY( immutable_matrix, AE_RESULT_INVALID_MEMORY );

        __make_movie_layer_transformation3d_immutable( *immutable_matrix, _transformation );

        _transformation->immutable_matrix = immutable_matrix;
    }
    else
    {
        _transformation->immutable_matrix = AE_NULL;
    }

    return AE_RESULT_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t __make_layer_transformation_interpolate_immutable( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index, ae_float_t _t )
{
    AE_UNUSED( _index );
    AE_UNUSED( _t );

    ae_copy_m4( _out, *_transformation->immutable_matrix );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t __make_layer_transformation_fixed_immutable( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index )
{
    AE_UNUSED( _index );

    ae_copy_m4( _out, *_transformation->immutable_matrix );
}
//////////////////////////////////////////////////////////////////////////
#define AE_INTERPOLATE_PROPERTY( Transformation, Name, OutName )\
	OutName = (Transformation->timeline == AE_NULL || Transformation->timeline->Name == AE_NULL) ? Transformation->immutable.Name : __get_movie_layer_transformation_property_interpolate(\
		Transformation->timeline->Name,\
		_index, _t )
//////////////////////////////////////////////////////////////////////////
#define AE_FIXED_PROPERTY( Transformation, Name, Index, OutName)\
	OutName = (Transformation->timeline == AE_NULL || Transformation->timeline->Name == AE_NULL) ? Transformation->immutable.Name : __get_movie_layer_transformation_property(\
		Transformation->timeline->Name,\
		_index + Index )
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t __make_layer_transformation2d_interpolate( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index, ae_float_t _t )
{
    const aeMovieLayerTransformation2D * transformation2d = (const aeMovieLayerTransformation2D *)_transformation;

    ae_vector2_t anchor_point;
    ae_vector2_t position;
    ae_vector2_t scale;
    ae_quaternionzw_t quaternion;
    ae_skew_t skew;

    AE_INTERPOLATE_PROPERTY( transformation2d, anchor_point_x, anchor_point[0] );
    AE_INTERPOLATE_PROPERTY( transformation2d, anchor_point_y, anchor_point[1] );

    AE_INTERPOLATE_PROPERTY( transformation2d, position_x, position[0] );
    AE_INTERPOLATE_PROPERTY( transformation2d, position_y, position[1] );

    AE_INTERPOLATE_PROPERTY( transformation2d, scale_x, scale[0] );
    AE_INTERPOLATE_PROPERTY( transformation2d, scale_y, scale[1] );

    ae_quaternionzw_t q1;
    AE_FIXED_PROPERTY( transformation2d, quaternion_z, 0, q1[0] );
    AE_FIXED_PROPERTY( transformation2d, quaternion_w, 0, q1[1] );

    ae_quaternionzw_t q2;
    AE_FIXED_PROPERTY( transformation2d, quaternion_z, 1, q2[0] );
    AE_FIXED_PROPERTY( transformation2d, quaternion_w, 1, q2[1] );

    ae_linerp_qzw( quaternion, q1, q2, _t );

    AE_INTERPOLATE_PROPERTY( transformation2d, skew, skew[0] );
    AE_INTERPOLATE_PROPERTY( transformation2d, skew_quaternion_z, skew[1] );
    AE_INTERPOLATE_PROPERTY( transformation2d, skew_quaternion_w, skew[2] );

    ae_movie_make_transformation2d_m4( _out, position, anchor_point, scale, quaternion, skew );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t __make_layer_transformation2d_interpolate_fq( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index, ae_float_t _t )
{
    const aeMovieLayerTransformation2D * transformation2d = (const aeMovieLayerTransformation2D *)_transformation;

    ae_vector2_t anchor_point;
    ae_vector2_t position;
    ae_vector2_t scale;
    ae_quaternionzw_t quaternion;
    ae_skew_t skew;

    AE_INTERPOLATE_PROPERTY( transformation2d, anchor_point_x, anchor_point[0] );
    AE_INTERPOLATE_PROPERTY( transformation2d, anchor_point_y, anchor_point[1] );

    AE_INTERPOLATE_PROPERTY( transformation2d, position_x, position[0] );
    AE_INTERPOLATE_PROPERTY( transformation2d, position_y, position[1] );

    AE_INTERPOLATE_PROPERTY( transformation2d, scale_x, scale[0] );
    AE_INTERPOLATE_PROPERTY( transformation2d, scale_y, scale[1] );

    AE_FIXED_PROPERTY( transformation2d, quaternion_z, 0, quaternion[0] );
    AE_FIXED_PROPERTY( transformation2d, quaternion_w, 0, quaternion[1] );

    AE_INTERPOLATE_PROPERTY( transformation2d, skew, skew[0] );
    AE_INTERPOLATE_PROPERTY( transformation2d, skew_quaternion_z, skew[1] );
    AE_INTERPOLATE_PROPERTY( transformation2d, skew_quaternion_w, skew[2] );

    ae_movie_make_transformation2d_m4( _out, position, anchor_point, scale, quaternion, skew );
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_void_t __make_layer_transformation2d_interpolate_wskfq( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index, ae_float_t _t )
{
    const aeMovieLayerTransformation2D * transformation2d = (const aeMovieLayerTransformation2D *)_transformation;

    ae_vector2_t anchor_point;
    ae_vector2_t position;
    ae_vector2_t scale;
    ae_quaternionzw_t quaternion;

    AE_INTERPOLATE_PROPERTY( transformation2d, anchor_point_x, anchor_point[0] );
    AE_INTERPOLATE_PROPERTY( transformation2d, anchor_point_y, anchor_point[1] );

    AE_INTERPOLATE_PROPERTY( transformation2d, position_x, position[0] );
    AE_INTERPOLATE_PROPERTY( transformation2d, position_y, position[1] );

    AE_INTERPOLATE_PROPERTY( transformation2d, scale_x, scale[0] );
    AE_INTERPOLATE_PROPERTY( transformation2d, scale_y, scale[1] );

    AE_FIXED_PROPERTY( transformation2d, quaternion_z, 0, quaternion[0] );
    AE_FIXED_PROPERTY( transformation2d, quaternion_w, 0, quaternion[1] );

    ae_movie_make_transformation2d_m4wsk( _out, position, anchor_point, scale, quaternion );
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_void_t __make_layer_transformation2d_fixed( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index )
{
    const aeMovieLayerTransformation2D * transformation2d = (const aeMovieLayerTransformation2D *)_transformation;

    ae_vector2_t anchor_point;
    ae_vector2_t position;
    ae_vector2_t scale;
    ae_quaternionzw_t quaternion;
    ae_skew_t skew;

    AE_FIXED_PROPERTY( transformation2d, anchor_point_x, 0, anchor_point[0] );
    AE_FIXED_PROPERTY( transformation2d, anchor_point_y, 0, anchor_point[1] );

    AE_FIXED_PROPERTY( transformation2d, position_x, 0, position[0] );
    AE_FIXED_PROPERTY( transformation2d, position_y, 0, position[1] );

    AE_FIXED_PROPERTY( transformation2d, scale_x, 0, scale[0] );
    AE_FIXED_PROPERTY( transformation2d, scale_y, 0, scale[1] );

    AE_FIXED_PROPERTY( transformation2d, quaternion_z, 0, quaternion[0] );
    AE_FIXED_PROPERTY( transformation2d, quaternion_w, 0, quaternion[1] );

    AE_FIXED_PROPERTY( transformation2d, skew, 0, skew[0] );
    AE_FIXED_PROPERTY( transformation2d, skew_quaternion_z, 0, skew[1] );
    AE_FIXED_PROPERTY( transformation2d, skew_quaternion_w, 0, skew[2] );

    ae_movie_make_transformation2d_m4( _out, position, anchor_point, scale, quaternion, skew );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t __make_layer_transformation2d_interpolate_wq( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index, ae_float_t _t )
{
    const aeMovieLayerTransformation2D * transformation2d = (const aeMovieLayerTransformation2D *)_transformation;

    ae_vector2_t anchor_point;
    ae_vector2_t position;
    ae_vector2_t scale;
    ae_skew_t skew;

    AE_INTERPOLATE_PROPERTY( transformation2d, anchor_point_x, anchor_point[0] );
    AE_INTERPOLATE_PROPERTY( transformation2d, anchor_point_y, anchor_point[1] );

    AE_INTERPOLATE_PROPERTY( transformation2d, position_x, position[0] );
    AE_INTERPOLATE_PROPERTY( transformation2d, position_y, position[1] );

    AE_INTERPOLATE_PROPERTY( transformation2d, scale_x, scale[0] );
    AE_INTERPOLATE_PROPERTY( transformation2d, scale_y, scale[1] );

    AE_INTERPOLATE_PROPERTY( transformation2d, skew, skew[0] );
    AE_INTERPOLATE_PROPERTY( transformation2d, skew_quaternion_z, skew[1] );
    AE_INTERPOLATE_PROPERTY( transformation2d, skew_quaternion_w, skew[2] );

    ae_movie_make_transformation2d_m4wq( _out, position, anchor_point, scale, skew );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t __make_layer_transformation2d_fixed_wq( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index )
{
    const aeMovieLayerTransformation2D * transformation2d = (const aeMovieLayerTransformation2D *)_transformation;

    ae_vector2_t anchor_point;
    ae_vector2_t position;
    ae_vector2_t scale;
    ae_skew_t skew;

    AE_FIXED_PROPERTY( transformation2d, anchor_point_x, 0, anchor_point[0] );
    AE_FIXED_PROPERTY( transformation2d, anchor_point_y, 0, anchor_point[1] );

    AE_FIXED_PROPERTY( transformation2d, position_x, 0, position[0] );
    AE_FIXED_PROPERTY( transformation2d, position_y, 0, position[1] );

    AE_FIXED_PROPERTY( transformation2d, scale_x, 0, scale[0] );
    AE_FIXED_PROPERTY( transformation2d, scale_y, 0, scale[1] );

    AE_FIXED_PROPERTY( transformation2d, skew, 0, skew[0] );
    AE_FIXED_PROPERTY( transformation2d, skew_quaternion_z, 0, skew[1] );
    AE_FIXED_PROPERTY( transformation2d, skew_quaternion_w, 0, skew[2] );

    ae_movie_make_transformation2d_m4wq( _out, position, anchor_point, scale, skew );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t __make_layer_transformation2d_interpolate_wskq( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index, ae_float_t _t )
{
    const aeMovieLayerTransformation2D * transformation2d = (const aeMovieLayerTransformation2D *)_transformation;

    ae_vector2_t anchor_point;
    ae_vector2_t position;
    ae_vector2_t scale;

    AE_INTERPOLATE_PROPERTY( transformation2d, anchor_point_x, anchor_point[0] );
    AE_INTERPOLATE_PROPERTY( transformation2d, anchor_point_y, anchor_point[1] );

    AE_INTERPOLATE_PROPERTY( transformation2d, position_x, position[0] );
    AE_INTERPOLATE_PROPERTY( transformation2d, position_y, position[1] );

    AE_INTERPOLATE_PROPERTY( transformation2d, scale_x, scale[0] );
    AE_INTERPOLATE_PROPERTY( transformation2d, scale_y, scale[1] );

    ae_movie_make_transformation2d_m4wskq( _out, position, anchor_point, scale );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t __make_layer_transformation2d_fixed_wskq( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index )
{
    const aeMovieLayerTransformation2D * transformation2d = (const aeMovieLayerTransformation2D *)_transformation;

    ae_vector2_t anchor_point;
    ae_vector2_t position;
    ae_vector2_t scale;

    AE_FIXED_PROPERTY( transformation2d, anchor_point_x, 0, anchor_point[0] );
    AE_FIXED_PROPERTY( transformation2d, anchor_point_y, 0, anchor_point[1] );

    AE_FIXED_PROPERTY( transformation2d, position_x, 0, position[0] );
    AE_FIXED_PROPERTY( transformation2d, position_y, 0, position[1] );

    AE_FIXED_PROPERTY( transformation2d, scale_x, 0, scale[0] );
    AE_FIXED_PROPERTY( transformation2d, scale_y, 0, scale[1] );

    ae_movie_make_transformation2d_m4wskq( _out, position, anchor_point, scale );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t __make_layer_transformation2d_interpolate_wsk( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index, ae_float_t _t )
{
    const aeMovieLayerTransformation2D * transformation2d = (const aeMovieLayerTransformation2D *)_transformation;

    ae_vector2_t anchor_point;
    ae_vector2_t position;
    ae_vector2_t scale;
    ae_quaternionzw_t quaternion;

    AE_INTERPOLATE_PROPERTY( transformation2d, anchor_point_x, anchor_point[0] );
    AE_INTERPOLATE_PROPERTY( transformation2d, anchor_point_y, anchor_point[1] );

    AE_INTERPOLATE_PROPERTY( transformation2d, position_x, position[0] );
    AE_INTERPOLATE_PROPERTY( transformation2d, position_y, position[1] );

    AE_INTERPOLATE_PROPERTY( transformation2d, scale_x, scale[0] );
    AE_INTERPOLATE_PROPERTY( transformation2d, scale_y, scale[1] );

    ae_quaternionzw_t q1;
    AE_FIXED_PROPERTY( transformation2d, quaternion_z, 0, q1[0] );
    AE_FIXED_PROPERTY( transformation2d, quaternion_w, 0, q1[1] );

    ae_quaternionzw_t q2;
    AE_FIXED_PROPERTY( transformation2d, quaternion_z, 1, q2[0] );
    AE_FIXED_PROPERTY( transformation2d, quaternion_w, 1, q2[1] );

    ae_linerp_qzw( quaternion, q1, q2, _t );

    ae_movie_make_transformation2d_m4wsk( _out, position, anchor_point, scale, quaternion );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t __make_layer_transformation2d_fixed_wsk( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index )
{
    const aeMovieLayerTransformation2D * transformation2d = (const aeMovieLayerTransformation2D *)_transformation;

    ae_vector2_t anchor_point;
    ae_vector2_t position;
    ae_vector2_t scale;
    ae_quaternionzw_t quaternion;

    AE_FIXED_PROPERTY( transformation2d, anchor_point_x, 0, anchor_point[0] );
    AE_FIXED_PROPERTY( transformation2d, anchor_point_y, 0, anchor_point[1] );

    AE_FIXED_PROPERTY( transformation2d, position_x, 0, position[0] );
    AE_FIXED_PROPERTY( transformation2d, position_y, 0, position[1] );

    AE_FIXED_PROPERTY( transformation2d, scale_x, 0, scale[0] );
    AE_FIXED_PROPERTY( transformation2d, scale_y, 0, scale[1] );

    AE_FIXED_PROPERTY( transformation2d, quaternion_z, 0, quaternion[0] );
    AE_FIXED_PROPERTY( transformation2d, quaternion_w, 0, quaternion[1] );

    ae_movie_make_transformation2d_m4wsk( _out, position, anchor_point, scale, quaternion );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t __make_layer_transformation3d_fixed( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index )
{
    const aeMovieLayerTransformation3D * transformation3d = (const aeMovieLayerTransformation3D *)_transformation;

    ae_vector3_t anchor_point;
    ae_vector3_t position;
    ae_vector3_t scale;
    ae_quaternion_t quaternion;
    ae_skew_t skew;

    AE_FIXED_PROPERTY( transformation3d, anchor_point_x, 0, anchor_point[0] );
    AE_FIXED_PROPERTY( transformation3d, anchor_point_y, 0, anchor_point[1] );
    AE_FIXED_PROPERTY( transformation3d, anchor_point_z, 0, anchor_point[2] );

    AE_FIXED_PROPERTY( transformation3d, position_x, 0, position[0] );
    AE_FIXED_PROPERTY( transformation3d, position_y, 0, position[1] );
    AE_FIXED_PROPERTY( transformation3d, position_z, 0, position[2] );

    AE_FIXED_PROPERTY( transformation3d, scale_x, 0, scale[0] );
    AE_FIXED_PROPERTY( transformation3d, scale_y, 0, scale[1] );
    AE_FIXED_PROPERTY( transformation3d, scale_z, 0, scale[2] );

    AE_FIXED_PROPERTY( transformation3d, quaternion_x, 0, quaternion[0] );
    AE_FIXED_PROPERTY( transformation3d, quaternion_y, 0, quaternion[1] );
    AE_FIXED_PROPERTY( transformation3d, quaternion_z, 0, quaternion[2] );
    AE_FIXED_PROPERTY( transformation3d, quaternion_w, 0, quaternion[3] );

    AE_FIXED_PROPERTY( transformation3d, skew, 0, skew[0] );
    AE_FIXED_PROPERTY( transformation3d, skew_quaternion_z, 0, skew[1] );
    AE_FIXED_PROPERTY( transformation3d, skew_quaternion_w, 0, skew[2] );

    ae_movie_make_transformation3d_m4( _out, position, anchor_point, scale, quaternion, skew );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t __make_layer_transformation3d_fixed_wq( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index )
{
    const aeMovieLayerTransformation3D * transformation3d = (const aeMovieLayerTransformation3D *)_transformation;

    ae_vector3_t anchor_point;
    ae_vector3_t position;
    ae_vector3_t scale;
    ae_skew_t skew;

    AE_FIXED_PROPERTY( transformation3d, anchor_point_x, 0, anchor_point[0] );
    AE_FIXED_PROPERTY( transformation3d, anchor_point_y, 0, anchor_point[1] );
    AE_FIXED_PROPERTY( transformation3d, anchor_point_z, 0, anchor_point[2] );

    AE_FIXED_PROPERTY( transformation3d, position_x, 0, position[0] );
    AE_FIXED_PROPERTY( transformation3d, position_y, 0, position[1] );
    AE_FIXED_PROPERTY( transformation3d, position_z, 0, position[2] );

    AE_FIXED_PROPERTY( transformation3d, scale_x, 0, scale[0] );
    AE_FIXED_PROPERTY( transformation3d, scale_y, 0, scale[1] );
    AE_FIXED_PROPERTY( transformation3d, scale_z, 0, scale[2] );

    AE_FIXED_PROPERTY( transformation3d, skew, 0, skew[0] );
    AE_FIXED_PROPERTY( transformation3d, skew_quaternion_z, 0, skew[1] );
    AE_FIXED_PROPERTY( transformation3d, skew_quaternion_w, 0, skew[2] );

    ae_movie_make_transformation3d_m4wq( _out, position, anchor_point, scale, skew );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t __make_layer_transformation3d_interpolate_wq( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index, ae_float_t _t )
{
    const aeMovieLayerTransformation3D * transformation3d = (const aeMovieLayerTransformation3D *)_transformation;

    ae_vector3_t anchor_point;
    ae_vector3_t position;
    ae_vector3_t scale;
    ae_skew_t skew;
    
    AE_INTERPOLATE_PROPERTY( transformation3d, anchor_point_x, anchor_point[0] );
    AE_INTERPOLATE_PROPERTY( transformation3d, anchor_point_y, anchor_point[1] );
    AE_INTERPOLATE_PROPERTY( transformation3d, anchor_point_z, anchor_point[2] );

    AE_INTERPOLATE_PROPERTY( transformation3d, position_x, position[0] );
    AE_INTERPOLATE_PROPERTY( transformation3d, position_y, position[1] );
    AE_INTERPOLATE_PROPERTY( transformation3d, position_z, position[2] );

    AE_INTERPOLATE_PROPERTY( transformation3d, scale_x, scale[0] );
    AE_INTERPOLATE_PROPERTY( transformation3d, scale_y, scale[1] );
    AE_INTERPOLATE_PROPERTY( transformation3d, scale_z, scale[2] );

    AE_INTERPOLATE_PROPERTY( transformation3d, skew, skew[0] );
    AE_INTERPOLATE_PROPERTY( transformation3d, skew_quaternion_z, skew[1] );
    AE_INTERPOLATE_PROPERTY( transformation3d, skew_quaternion_w, skew[2] );

    ae_movie_make_transformation3d_m4wq( _out, position, anchor_point, scale, skew );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t __make_layer_transformation3d_interpolate_wskq( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index, ae_float_t _t )
{
    const aeMovieLayerTransformation3D * transformation3d = (const aeMovieLayerTransformation3D *)_transformation;

    ae_vector3_t anchor_point;
    ae_vector3_t position;
    ae_vector3_t scale;

    AE_INTERPOLATE_PROPERTY( transformation3d, anchor_point_x, anchor_point[0] );
    AE_INTERPOLATE_PROPERTY( transformation3d, anchor_point_y, anchor_point[1] );
    AE_INTERPOLATE_PROPERTY( transformation3d, anchor_point_z, anchor_point[2] );

    AE_INTERPOLATE_PROPERTY( transformation3d, position_x, position[0] );
    AE_INTERPOLATE_PROPERTY( transformation3d, position_y, position[1] );
    AE_INTERPOLATE_PROPERTY( transformation3d, position_z, position[2] );

    AE_INTERPOLATE_PROPERTY( transformation3d, scale_x, scale[0] );
    AE_INTERPOLATE_PROPERTY( transformation3d, scale_y, scale[1] );
    AE_INTERPOLATE_PROPERTY( transformation3d, scale_z, scale[2] );

    ae_movie_make_transformation3d_m4wskq( _out, position, anchor_point, scale );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t __make_layer_transformation3d_fixed_wskq( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index )
{
    const aeMovieLayerTransformation3D * transformation3d = (const aeMovieLayerTransformation3D *)_transformation;

    ae_vector3_t anchor_point;
    ae_vector3_t position;
    ae_vector3_t scale;

    AE_FIXED_PROPERTY( transformation3d, anchor_point_x, 0, anchor_point[0] );
    AE_FIXED_PROPERTY( transformation3d, anchor_point_y, 0, anchor_point[1] );
    AE_FIXED_PROPERTY( transformation3d, anchor_point_z, 0, anchor_point[2] );

    AE_FIXED_PROPERTY( transformation3d, position_x, 0, position[0] );
    AE_FIXED_PROPERTY( transformation3d, position_y, 0, position[1] );
    AE_FIXED_PROPERTY( transformation3d, position_z, 0, position[2] );

    AE_FIXED_PROPERTY( transformation3d, scale_x, 0, scale[0] );
    AE_FIXED_PROPERTY( transformation3d, scale_y, 0, scale[1] );
    AE_FIXED_PROPERTY( transformation3d, scale_z, 0, scale[2] );

    ae_movie_make_transformation3d_m4wskq( _out, position, anchor_point, scale );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t __make_layer_transformation3d_interpolate_wsk( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index, ae_float_t _t )
{
    const aeMovieLayerTransformation3D * transformation3d = (const aeMovieLayerTransformation3D *)_transformation;

    ae_vector3_t anchor_point;
    ae_vector3_t position;
    ae_vector3_t scale;
    ae_quaternion_t quaternion;

    AE_INTERPOLATE_PROPERTY( transformation3d, anchor_point_x, anchor_point[0] );
    AE_INTERPOLATE_PROPERTY( transformation3d, anchor_point_y, anchor_point[1] );
    AE_INTERPOLATE_PROPERTY( transformation3d, anchor_point_z, anchor_point[2] );

    AE_INTERPOLATE_PROPERTY( transformation3d, position_x, position[0] );
    AE_INTERPOLATE_PROPERTY( transformation3d, position_y, position[1] );
    AE_INTERPOLATE_PROPERTY( transformation3d, position_z, position[2] );

    AE_INTERPOLATE_PROPERTY( transformation3d, scale_x, scale[0] );
    AE_INTERPOLATE_PROPERTY( transformation3d, scale_y, scale[1] );
    AE_INTERPOLATE_PROPERTY( transformation3d, scale_z, scale[2] );

    ae_quaternion_t q1;
    AE_FIXED_PROPERTY( transformation3d, quaternion_x, 0, q1[0] );
    AE_FIXED_PROPERTY( transformation3d, quaternion_y, 0, q1[1] );
    AE_FIXED_PROPERTY( transformation3d, quaternion_z, 0, q1[2] );
    AE_FIXED_PROPERTY( transformation3d, quaternion_w, 0, q1[3] );

    ae_quaternion_t q2;
    AE_FIXED_PROPERTY( transformation3d, quaternion_x, 1, q2[0] );
    AE_FIXED_PROPERTY( transformation3d, quaternion_y, 1, q2[1] );
    AE_FIXED_PROPERTY( transformation3d, quaternion_z, 1, q2[2] );
    AE_FIXED_PROPERTY( transformation3d, quaternion_w, 1, q2[3] );

    ae_linerp_q( quaternion, q1, q2, _t );

    ae_movie_make_transformation3d_m4wsk( _out, position, anchor_point, scale, quaternion );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t __make_layer_transformation3d_interpolate_wskfq( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index, ae_float_t _t )
{
    const aeMovieLayerTransformation3D * transformation3d = (const aeMovieLayerTransformation3D *)_transformation;

    ae_vector3_t anchor_point;
    ae_vector3_t position;
    ae_vector3_t scale;
    ae_quaternion_t quaternion;

    AE_INTERPOLATE_PROPERTY( transformation3d, anchor_point_x, anchor_point[0] );
    AE_INTERPOLATE_PROPERTY( transformation3d, anchor_point_y, anchor_point[1] );
    AE_INTERPOLATE_PROPERTY( transformation3d, anchor_point_z, anchor_point[2] );

    AE_INTERPOLATE_PROPERTY( transformation3d, position_x, position[0] );
    AE_INTERPOLATE_PROPERTY( transformation3d, position_y, position[1] );
    AE_INTERPOLATE_PROPERTY( transformation3d, position_z, position[2] );

    AE_INTERPOLATE_PROPERTY( transformation3d, scale_x, scale[0] );
    AE_INTERPOLATE_PROPERTY( transformation3d, scale_y, scale[1] );
    AE_INTERPOLATE_PROPERTY( transformation3d, scale_z, scale[2] );

    AE_FIXED_PROPERTY( transformation3d, quaternion_x, 0, quaternion[0] );
    AE_FIXED_PROPERTY( transformation3d, quaternion_y, 0, quaternion[1] );
    AE_FIXED_PROPERTY( transformation3d, quaternion_z, 0, quaternion[2] );
    AE_FIXED_PROPERTY( transformation3d, quaternion_w, 0, quaternion[3] );

    ae_movie_make_transformation3d_m4wsk( _out, position, anchor_point, scale, quaternion );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t __make_layer_transformation3d_fixed_wsk( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index )
{
    const aeMovieLayerTransformation3D * transformation3d = (const aeMovieLayerTransformation3D *)_transformation;

    ae_vector3_t anchor_point;
    ae_vector3_t position;
    ae_vector3_t scale;
    ae_quaternion_t quaternion;

    AE_FIXED_PROPERTY( transformation3d, anchor_point_x, 0, anchor_point[0] );
    AE_FIXED_PROPERTY( transformation3d, anchor_point_y, 0, anchor_point[1] );
    AE_FIXED_PROPERTY( transformation3d, anchor_point_z, 0, anchor_point[2] );

    AE_FIXED_PROPERTY( transformation3d, position_x, 0, position[0] );
    AE_FIXED_PROPERTY( transformation3d, position_y, 0, position[1] );
    AE_FIXED_PROPERTY( transformation3d, position_z, 0, position[2] );

    AE_FIXED_PROPERTY( transformation3d, scale_x, 0, scale[0] );
    AE_FIXED_PROPERTY( transformation3d, scale_y, 0, scale[1] );
    AE_FIXED_PROPERTY( transformation3d, scale_z, 0, scale[2] );

    AE_FIXED_PROPERTY( transformation3d, quaternion_x, 0, quaternion[0] );
    AE_FIXED_PROPERTY( transformation3d, quaternion_y, 0, quaternion[1] );
    AE_FIXED_PROPERTY( transformation3d, quaternion_z, 0, quaternion[2] );
    AE_FIXED_PROPERTY( transformation3d, quaternion_w, 0, quaternion[3] );

    ae_movie_make_transformation3d_m4wsk( _out, position, anchor_point, scale, quaternion );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t __make_layer_transformation3d_interpolate_fq( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index, ae_float_t _t )
{
    const aeMovieLayerTransformation3D * transformation3d = (const aeMovieLayerTransformation3D *)_transformation;

    ae_vector3_t anchor_point;
    ae_vector3_t position;
    ae_vector3_t scale;
    ae_quaternion_t quaternion;
    ae_skew_t skew;

    AE_INTERPOLATE_PROPERTY( transformation3d, anchor_point_x, anchor_point[0] );
    AE_INTERPOLATE_PROPERTY( transformation3d, anchor_point_y, anchor_point[1] );
    AE_INTERPOLATE_PROPERTY( transformation3d, anchor_point_z, anchor_point[2] );

    AE_INTERPOLATE_PROPERTY( transformation3d, position_x, position[0] );
    AE_INTERPOLATE_PROPERTY( transformation3d, position_y, position[1] );
    AE_INTERPOLATE_PROPERTY( transformation3d, position_z, position[2] );

    AE_INTERPOLATE_PROPERTY( transformation3d, scale_x, scale[0] );
    AE_INTERPOLATE_PROPERTY( transformation3d, scale_y, scale[1] );
    AE_INTERPOLATE_PROPERTY( transformation3d, scale_z, scale[2] );

    AE_FIXED_PROPERTY( transformation3d, quaternion_x, 0, quaternion[0] );
    AE_FIXED_PROPERTY( transformation3d, quaternion_y, 0, quaternion[1] );
    AE_FIXED_PROPERTY( transformation3d, quaternion_z, 0, quaternion[2] );
    AE_FIXED_PROPERTY( transformation3d, quaternion_w, 0, quaternion[3] );

    AE_INTERPOLATE_PROPERTY( transformation3d, skew, skew[0] );
    AE_INTERPOLATE_PROPERTY( transformation3d, skew_quaternion_z, skew[1] );
    AE_INTERPOLATE_PROPERTY( transformation3d, skew_quaternion_w, skew[2] );

    ae_movie_make_transformation3d_m4( _out, position, anchor_point, scale, quaternion, skew );
}
//////////////////////////////////////////////////////////////////////////
AE_CALLBACK ae_void_t __make_layer_transformation3d_interpolate( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index, ae_float_t _t )
{
    const aeMovieLayerTransformation3D * transformation3d = (const aeMovieLayerTransformation3D *)_transformation;

    ae_vector3_t anchor_point;
    ae_vector3_t position;
    ae_vector3_t scale;
    ae_quaternion_t quaternion;
    ae_skew_t skew;

    AE_INTERPOLATE_PROPERTY( transformation3d, anchor_point_x, anchor_point[0] );
    AE_INTERPOLATE_PROPERTY( transformation3d, anchor_point_y, anchor_point[1] );
    AE_INTERPOLATE_PROPERTY( transformation3d, anchor_point_z, anchor_point[2] );

    AE_INTERPOLATE_PROPERTY( transformation3d, position_x, position[0] );
    AE_INTERPOLATE_PROPERTY( transformation3d, position_y, position[1] );
    AE_INTERPOLATE_PROPERTY( transformation3d, position_z, position[2] );

    AE_INTERPOLATE_PROPERTY( transformation3d, scale_x, scale[0] );
    AE_INTERPOLATE_PROPERTY( transformation3d, scale_y, scale[1] );
    AE_INTERPOLATE_PROPERTY( transformation3d, scale_z, scale[2] );

    ae_quaternion_t q1;
    AE_FIXED_PROPERTY( transformation3d, quaternion_x, 0, q1[0] );
    AE_FIXED_PROPERTY( transformation3d, quaternion_y, 0, q1[1] );
    AE_FIXED_PROPERTY( transformation3d, quaternion_z, 0, q1[2] );
    AE_FIXED_PROPERTY( transformation3d, quaternion_w, 0, q1[3] );

    ae_quaternion_t q2;
    AE_FIXED_PROPERTY( transformation3d, quaternion_x, 1, q2[0] );
    AE_FIXED_PROPERTY( transformation3d, quaternion_y, 1, q2[1] );
    AE_FIXED_PROPERTY( transformation3d, quaternion_z, 1, q2[2] );
    AE_FIXED_PROPERTY( transformation3d, quaternion_w, 1, q2[3] );

    ae_linerp_q( quaternion, q1, q2, _t );

    AE_INTERPOLATE_PROPERTY( transformation3d, skew, skew[0] );
    AE_INTERPOLATE_PROPERTY( transformation3d, skew_quaternion_z, skew[1] );
    AE_INTERPOLATE_PROPERTY( transformation3d, skew_quaternion_w, skew[2] );

    ae_movie_make_transformation3d_m4( _out, position, anchor_point, scale, quaternion, skew );
}
//////////////////////////////////////////////////////////////////////////
ae_result_t ae_movie_load_layer_transformation( aeMovieStream * _stream, aeMovieLayerTransformation * _transformation, ae_bool_t _threeD )
{
    ae_uint32_t immutable_property_mask;
    AE_READ( _stream, immutable_property_mask );

    _transformation->immutable_property_mask = immutable_property_mask;

    if( _threeD == AE_FALSE )
    {
        aeMovieLayerTransformation2D * transformation2d = (aeMovieLayerTransformation2D *)_transformation;

        aeMovieLayerTransformation2DTimeline * timeline = AE_NULL;

        if( (immutable_property_mask & AE_MOVIE_IMMUTABLE_SUPER_TWO_D_ALL) != AE_MOVIE_IMMUTABLE_SUPER_TWO_D_ALL )
        {
            timeline = AE_NEW( _stream->instance, aeMovieLayerTransformation2DTimeline );

            AE_RESULT_PANIC_MEMORY( timeline );
        }
        
        transformation2d->timeline = timeline;

        AE_RESULT( __load_movie_layer_transformation2d, (_stream, immutable_property_mask, transformation2d) );

        if( (immutable_property_mask & AE_MOVIE_IMMUTABLE_SUPER_TWO_D_ALL) == AE_MOVIE_IMMUTABLE_SUPER_TWO_D_ALL )
        {
            transformation2d->transforamtion_interpolate_matrix = &__make_layer_transformation_interpolate_immutable;
            transformation2d->transforamtion_fixed_matrix = &__make_layer_transformation_fixed_immutable;
        }
        else
        {            
            ae_uint32_t fixed_transformation = 0;

            if( (immutable_property_mask & AE_MOVIE_IMMUTABLE_SUPER_ALL_D_SKEW) == AE_MOVIE_IMMUTABLE_SUPER_ALL_D_SKEW )
            {
                ae_float_t skew = transformation2d->immutable.skew;

                if( skew == 0.f )
                {
                    fixed_transformation += 0x00000001;
                }
            }

            if( (immutable_property_mask & AE_MOVIE_IMMUTABLE_SUPER_TWO_D_QUATERNION) == AE_MOVIE_IMMUTABLE_SUPER_TWO_D_QUATERNION )
            {
                ae_float_t qw = transformation2d->immutable.quaternion_w;

                if( qw == 1.f )
                {
                    fixed_transformation += 0x00000002;
                }
                else
                {
                    fixed_transformation += 0x00000004;
                }
            }

            switch( fixed_transformation )
            {
            case 0:
                {
                    transformation2d->transforamtion_interpolate_matrix = &__make_layer_transformation2d_interpolate;
                    transformation2d->transforamtion_fixed_matrix = &__make_layer_transformation2d_fixed;
                }break;
            case 1:
                {
                    transformation2d->transforamtion_interpolate_matrix = &__make_layer_transformation2d_interpolate_wsk;
                    transformation2d->transforamtion_fixed_matrix = &__make_layer_transformation2d_fixed_wsk;
                }break;
            case 2:
                {
                    transformation2d->transforamtion_interpolate_matrix = &__make_layer_transformation2d_interpolate_wq;
                    transformation2d->transforamtion_fixed_matrix = &__make_layer_transformation2d_fixed_wq;
                }break;
            case 3:
                {
                    transformation2d->transforamtion_interpolate_matrix = &__make_layer_transformation2d_interpolate_wskq;
                    transformation2d->transforamtion_fixed_matrix = &__make_layer_transformation2d_fixed_wskq;
                }break;
            case 4:
                {
                    transformation2d->transforamtion_interpolate_matrix = &__make_layer_transformation2d_interpolate_fq;
                    transformation2d->transforamtion_fixed_matrix = &__make_layer_transformation2d_fixed;
                }break;
            case 5:
                {
                    transformation2d->transforamtion_interpolate_matrix = &__make_layer_transformation2d_interpolate_wskfq;
                    transformation2d->transforamtion_fixed_matrix = &__make_layer_transformation2d_fixed_wsk;
                }break;
            default:
                {
                    return AE_RESULT_INTERNAL_ERROR;
                }break;
            }
        }
    }
    else
    {
        aeMovieLayerTransformation3D * transformation3d = (aeMovieLayerTransformation3D *)_transformation;

        aeMovieLayerTransformation3DTimeline * timeline = AE_NULL;

        if( (immutable_property_mask & AE_MOVIE_IMMUTABLE_SUPER_THREE_D_ALL) != AE_MOVIE_IMMUTABLE_SUPER_THREE_D_ALL )
        {
            timeline = AE_NEW( _stream->instance, aeMovieLayerTransformation3DTimeline );

            AE_RESULT_PANIC_MEMORY( timeline );
        }

        transformation3d->timeline = timeline;

        AE_RESULT( __load_movie_layer_transformation3d, (_stream, immutable_property_mask, transformation3d) );

        if( (immutable_property_mask & AE_MOVIE_IMMUTABLE_SUPER_THREE_D_ALL) == AE_MOVIE_IMMUTABLE_SUPER_THREE_D_ALL )
        {
            transformation3d->transforamtion_interpolate_matrix = &__make_layer_transformation_interpolate_immutable;
            transformation3d->transforamtion_fixed_matrix = &__make_layer_transformation_fixed_immutable;
        }
        else
        {
            ae_uint32_t fixed_transformation = 0;

            if( (immutable_property_mask & AE_MOVIE_IMMUTABLE_SUPER_ALL_D_SKEW) == AE_MOVIE_IMMUTABLE_SUPER_ALL_D_SKEW )
            {                
                ae_float_t skew = transformation3d->immutable.skew;

                if( skew == 0.f )
                {
                    fixed_transformation += 0x00000001;
                }
            }

            if( (immutable_property_mask & AE_MOVIE_IMMUTABLE_SUPER_THREE_D_QUATERNION) == AE_MOVIE_IMMUTABLE_SUPER_THREE_D_QUATERNION )
            {
                ae_float_t qw = transformation3d->immutable.quaternion_w;

                if( qw == 1.f )
                {
                    fixed_transformation += 0x00000002;
                }
                else
                {
                    fixed_transformation += 0x00000004;
                }
            }

            switch( fixed_transformation )
            {
            case 0:
                {
                    transformation3d->transforamtion_interpolate_matrix = &__make_layer_transformation3d_interpolate;
                    transformation3d->transforamtion_fixed_matrix = &__make_layer_transformation3d_fixed;
                }break;
            case 1:
                {
                    transformation3d->transforamtion_interpolate_matrix = &__make_layer_transformation3d_interpolate_wsk;
                    transformation3d->transforamtion_fixed_matrix = &__make_layer_transformation3d_fixed_wsk;
                }break;
            case 2:
                {
                    transformation3d->transforamtion_interpolate_matrix = &__make_layer_transformation3d_interpolate_wq;
                    transformation3d->transforamtion_fixed_matrix = &__make_layer_transformation3d_fixed_wq;
                }break;
            case 3:
                {
                    transformation3d->transforamtion_interpolate_matrix = &__make_layer_transformation3d_interpolate_wskq;
                    transformation3d->transforamtion_fixed_matrix = &__make_layer_transformation3d_fixed_wskq;
                }break;
            case 4:
                {
                    transformation3d->transforamtion_interpolate_matrix = &__make_layer_transformation3d_interpolate_fq;
                    transformation3d->transforamtion_fixed_matrix = &__make_layer_transformation3d_fixed;
                }break;
            case 5:
                {
                    transformation3d->transforamtion_interpolate_matrix = &__make_layer_transformation3d_interpolate_wskfq;
                    transformation3d->transforamtion_fixed_matrix = &__make_layer_transformation3d_fixed_wsk;
                }break;
            default:
                {
                    return AE_RESULT_INTERNAL_ERROR;
                }break;
            };
        }
    }

    if( immutable_property_mask & AE_MOVIE_IMMUTABLE_OPACITY )
    {
        AE_READF( _stream, _transformation->immutable_opacity );
        _transformation->timeline_opacity = AE_NULL;
    }
    else
    {
        _transformation->immutable_opacity = 0.f;
        _transformation->timeline_opacity = __load_movie_layer_transformation_timeline( _stream, "immutable_opacity" );
    }

    return AE_RESULT_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_result_t __load_camera_transformation_property( aeMovieStream * _stream, ae_uint32_t _mask, aeMovieCompositionCamera * _transformation )
{
    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_TARGET_X, target_x );
    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_TARGET_Y, target_y );
    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_TARGET_Z, target_z );

    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_X, position_x );
    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_Y, position_y );
    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_Z, position_z );

    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_X, quaternion_x );
    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_Y, quaternion_y );
    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_Z, quaternion_z );
    AE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_W, quaternion_w );

    return AE_RESULT_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ae_result_t ae_movie_load_camera_transformation( aeMovieStream * _stream, aeMovieCompositionCamera * _camera )
{
    ae_uint32_t immutable_property_mask;
    AE_READ( _stream, immutable_property_mask );

    _camera->immutable_property_mask = immutable_property_mask;

    aeMovieCompositionCameraTimeline * timeline = AE_NULL;

    if( (immutable_property_mask & AE_MOVIE_IMMUTABLE_SUPER_CAMERA_ALL) != AE_MOVIE_IMMUTABLE_SUPER_CAMERA_ALL )
    {
        timeline = AE_NEW( _stream->instance, aeMovieCompositionCameraTimeline );

        AE_RESULT_PANIC_MEMORY( timeline );
    }

    _camera->timeline = timeline;

    AE_RESULT( __load_camera_transformation_property, (_stream, immutable_property_mask, _camera) );

    return AE_RESULT_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_void_t __delete_layer_transformation2d( const aeMovieInstance * _instance, const aeMovieLayerTransformation2D * _transformation )
{
    if( _transformation->timeline != AE_NULL )
    {
        aeMovieLayerTransformation2DTimeline * timeline = _transformation->timeline;

        AE_DELETE( _instance, timeline->anchor_point_x );
        AE_DELETE( _instance, timeline->anchor_point_y );
        AE_DELETE( _instance, timeline->position_x );
        AE_DELETE( _instance, timeline->position_y );
        AE_DELETE( _instance, timeline->scale_x );
        AE_DELETE( _instance, timeline->scale_y );
        AE_DELETE( _instance, timeline->quaternion_z );
        AE_DELETE( _instance, timeline->quaternion_w );
        AE_DELETE( _instance, timeline->skew );
        AE_DELETE( _instance, timeline->skew_quaternion_z );
        AE_DELETE( _instance, timeline->skew_quaternion_w );

        AE_DELETE( _instance, _transformation->timeline );
    }
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_void_t __delete_layer_transformation3d( const aeMovieInstance * _instance, const aeMovieLayerTransformation3D * _transformation )
{
    if( _transformation->timeline != AE_NULL )
    {
        aeMovieLayerTransformation3DTimeline * timeline = _transformation->timeline;

        AE_DELETE( _instance, timeline->anchor_point_x );
        AE_DELETE( _instance, timeline->anchor_point_y );
        AE_DELETE( _instance, timeline->anchor_point_z );
        AE_DELETE( _instance, timeline->position_x );
        AE_DELETE( _instance, timeline->position_y );
        AE_DELETE( _instance, timeline->position_z );
        AE_DELETE( _instance, timeline->scale_x );
        AE_DELETE( _instance, timeline->scale_y );
        AE_DELETE( _instance, timeline->scale_z );
        AE_DELETE( _instance, timeline->quaternion_x );
        AE_DELETE( _instance, timeline->quaternion_y );
        AE_DELETE( _instance, timeline->quaternion_z );
        AE_DELETE( _instance, timeline->quaternion_w );
        AE_DELETE( _instance, timeline->skew );
        AE_DELETE( _instance, timeline->skew_quaternion_z );
        AE_DELETE( _instance, timeline->skew_quaternion_w );

        AE_DELETE( _instance, _transformation->timeline );
    }
}
//////////////////////////////////////////////////////////////////////////
ae_void_t ae_movie_delete_layer_transformation( const aeMovieInstance * _instance, const aeMovieLayerTransformation * _transformation, ae_bool_t _threeD )
{
    AE_DELETE( _instance, _transformation->timeline_opacity );

    if( _threeD == AE_FALSE )
    {
        __delete_layer_transformation2d( _instance, (const aeMovieLayerTransformation2D *)_transformation );
    }
    else
    {
        __delete_layer_transformation3d( _instance, (const aeMovieLayerTransformation3D *)_transformation );
    }

    if( _transformation->immutable_matrix != AE_NULL )
    {
        AE_DELETE( _instance, _transformation->immutable_matrix );
    }
}
//////////////////////////////////////////////////////////////////////////
ae_void_t ae_movie_make_layer_matrix( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_bool_t _interpolate, ae_uint32_t _index, ae_float_t _t )
{
    if( _interpolate == AE_TRUE )
    {
        (*_transformation->transforamtion_interpolate_matrix)(_out, _transformation, _index, _t);
    }
    else
    {
        (*_transformation->transforamtion_fixed_matrix)(_out, _transformation, _index);
    }
}
//////////////////////////////////////////////////////////////////////////
ae_void_t ae_movie_make_camera_transformation( ae_vector3_t _target, ae_vector3_t _position, ae_quaternion_t _quaternion, const aeMovieCompositionCamera * _camera, ae_uint32_t _index, ae_bool_t _interpolate, ae_float_t _t )
{
    if( _interpolate == AE_TRUE )
    {
        AE_INTERPOLATE_PROPERTY( _camera, target_x, _target[0] );
        AE_INTERPOLATE_PROPERTY( _camera, target_y, _target[1] );
        AE_INTERPOLATE_PROPERTY( _camera, target_z, _target[2] );

        AE_INTERPOLATE_PROPERTY( _camera, position_x, _position[0] );
        AE_INTERPOLATE_PROPERTY( _camera, position_y, _position[1] );
        AE_INTERPOLATE_PROPERTY( _camera, position_z, _position[2] );

        ae_quaternion_t q1;
        AE_FIXED_PROPERTY( _camera, quaternion_x, 0, q1[0] );
        AE_FIXED_PROPERTY( _camera, quaternion_y, 0, q1[1] );
        AE_FIXED_PROPERTY( _camera, quaternion_z, 0, q1[2] );
        AE_FIXED_PROPERTY( _camera, quaternion_w, 0, q1[3] );

        ae_quaternion_t q2;
        AE_FIXED_PROPERTY( _camera, quaternion_x, 1, q2[0] );
        AE_FIXED_PROPERTY( _camera, quaternion_y, 1, q2[1] );
        AE_FIXED_PROPERTY( _camera, quaternion_z, 1, q2[2] );
        AE_FIXED_PROPERTY( _camera, quaternion_w, 1, q2[3] );

        ae_linerp_q( _quaternion, q1, q2, _t );
    }
    else
    {
        AE_FIXED_PROPERTY( _camera, target_x, 0, _target[0] );
        AE_FIXED_PROPERTY( _camera, target_y, 0, _target[1] );
        AE_FIXED_PROPERTY( _camera, target_z, 0, _target[2] );

        AE_FIXED_PROPERTY( _camera, position_x, 0, _position[0] );
        AE_FIXED_PROPERTY( _camera, position_y, 0, _position[1] );
        AE_FIXED_PROPERTY( _camera, position_z, 0, _position[2] );

        AE_FIXED_PROPERTY( _camera, quaternion_x, 0, _quaternion[0] );
        AE_FIXED_PROPERTY( _camera, quaternion_y, 0, _quaternion[1] );
        AE_FIXED_PROPERTY( _camera, quaternion_z, 0, _quaternion[2] );
        AE_FIXED_PROPERTY( _camera, quaternion_w, 0, _quaternion[3] );
    }
}
//////////////////////////////////////////////////////////////////////////
ae_color_channel_t ae_movie_make_layer_opacity( const aeMovieLayerTransformation * _transformation, ae_uint32_t _index, ae_bool_t _interpolate, ae_float_t _t )
{
    if( _transformation->timeline_opacity == AE_NULL )
    {
        return _transformation->immutable_opacity;
    }

    ae_color_channel_t opacity;

    if( _interpolate == AE_TRUE )
    {
        opacity = __get_movie_layer_transformation_property_interpolate( _transformation->timeline_opacity, _index, _t );
    }
    else
    {
        opacity = __get_movie_layer_transformation_property( _transformation->timeline_opacity, _index );
    }

    return opacity;
}
//////////////////////////////////////////////////////////////////////////
ae_void_t ae_movie_make_layer_transformation2d_interpolate( ae_vector2_t _anchor_point, ae_vector2_t _position, ae_vector2_t _scale, ae_quaternionzw_t _quaternion, ae_skew_t _skew, const aeMovieLayerTransformation2D * _transformation2d, ae_uint32_t _index, ae_float_t _t )
{
    AE_INTERPOLATE_PROPERTY( _transformation2d, anchor_point_x, _anchor_point[0] );
    AE_INTERPOLATE_PROPERTY( _transformation2d, anchor_point_y, _anchor_point[1] );

    AE_INTERPOLATE_PROPERTY( _transformation2d, position_x, _position[0] );
    AE_INTERPOLATE_PROPERTY( _transformation2d, position_y, _position[1] );

    AE_INTERPOLATE_PROPERTY( _transformation2d, scale_x, _scale[0] );
    AE_INTERPOLATE_PROPERTY( _transformation2d, scale_y, _scale[1] );

    ae_quaternionzw_t q1;
    AE_FIXED_PROPERTY( _transformation2d, quaternion_z, 0, q1[0] );
    AE_FIXED_PROPERTY( _transformation2d, quaternion_w, 0, q1[1] );

    ae_quaternionzw_t q2;
    AE_FIXED_PROPERTY( _transformation2d, quaternion_z, 1, q2[0] );
    AE_FIXED_PROPERTY( _transformation2d, quaternion_w, 1, q2[1] );

    ae_linerp_qzw( _quaternion, q1, q2, _t );

    AE_INTERPOLATE_PROPERTY( _transformation2d, skew, _skew[0] );
    AE_INTERPOLATE_PROPERTY( _transformation2d, skew_quaternion_z, _skew[1] );
    AE_INTERPOLATE_PROPERTY( _transformation2d, skew_quaternion_w, _skew[2] );
}
//////////////////////////////////////////////////////////////////////////
ae_void_t ae_movie_make_layer_transformation2d_fixed( ae_vector2_t _anchor_point, ae_vector2_t _position, ae_vector2_t _scale, ae_quaternionzw_t _quaternion, ae_skew_t _skew, const aeMovieLayerTransformation2D * _transformation2d, ae_uint32_t _index )
{
    AE_FIXED_PROPERTY( _transformation2d, anchor_point_x, 0, _anchor_point[0] );
    AE_FIXED_PROPERTY( _transformation2d, anchor_point_y, 0, _anchor_point[1] );

    AE_FIXED_PROPERTY( _transformation2d, position_x, 0, _position[0] );
    AE_FIXED_PROPERTY( _transformation2d, position_y, 0, _position[1] );

    AE_FIXED_PROPERTY( _transformation2d, scale_x, 0, _scale[0] );
    AE_FIXED_PROPERTY( _transformation2d, scale_y, 0, _scale[1] );

    AE_FIXED_PROPERTY( _transformation2d, quaternion_z, 0, _quaternion[0] );
    AE_FIXED_PROPERTY( _transformation2d, quaternion_w, 0, _quaternion[1] );

    AE_FIXED_PROPERTY( _transformation2d, skew, 0, _skew[0] );
    AE_FIXED_PROPERTY( _transformation2d, skew_quaternion_z, 0, _skew[1] );
    AE_FIXED_PROPERTY( _transformation2d, skew_quaternion_w, 0, _skew[2] );
}