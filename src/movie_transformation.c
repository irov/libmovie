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

#	include "movie_transformation.h"

#	include "movie_stream.h"
#	include "movie_math.h"

//////////////////////////////////////////////////////////////////////////
static ae_constvoidptr_t __load_movie_layer_transformation_timeline( aeMovieStream * _stream, const ae_char_t * _doc )
{
    (void)_doc;

    ae_uint32_t zp_data_size;
    READ( _stream, zp_data_size );

#	ifndef _DEBUG
    if( zp_data_size % 4 != 0 )
    {
        return AE_NULL;
    }
#	endif

    ae_uint32_t hashmask_iterator;
    READ( _stream, hashmask_iterator );

    ae_voidptr_t timeline = AE_NEWV( _stream->instance, _doc, zp_data_size );
    READV( _stream, timeline, (ae_size_t)zp_data_size );

    const ae_uint32_t * hashmask = _stream->instance->hashmask;

    for( ae_uint32_t
        *it = (ae_uint32_t *)timeline,
        *it_end = (ae_uint32_t *)timeline + zp_data_size / 4;
        it != it_end;
        ++it )
    {
        ae_uint32_t hashmask_index = hashmask_iterator++ % 5;

        ae_uint32_t hash = hashmask[hashmask_index];

        *it ^= hash;
    }

    return timeline;
}
//////////////////////////////////////////////////////////////////////////
static ae_float_t __get_movie_transformation_property( ae_constvoidptr_t _property, ae_uint32_t _index )
{
    ae_uint32_t property_index = 0;

    const ae_uint32_t * property_ae_uint32_t = (const ae_uint32_t *)_property;

    ae_uint32_t zp_count = *(property_ae_uint32_t++);

    ae_uint32_t i = 0;
    for( ; i != zp_count; ++i )
    {
        ae_uint32_t zp_block_type_count_data = *(property_ae_uint32_t++);

        ae_uint32_t zp_block_type = zp_block_type_count_data >> 24;
        ae_uint32_t zp_block_count = zp_block_type_count_data & 0x00FFFFFF;

        if( property_index + zp_block_count > _index )
        {
            switch( zp_block_type )
            {
            case 0:
                {
                    ae_float_t block_value = *(const ae_float_t *)(ae_constvoidptr_t)(property_ae_uint32_t);

                    return block_value;
                }break;
            case 1:
                {
                    ae_float_t block_begin = *(const ae_float_t *)(ae_constvoidptr_t)(property_ae_uint32_t++);
                    ae_float_t block_end = *(const ae_float_t *)(ae_constvoidptr_t)(property_ae_uint32_t);

                    ae_float_t block_add = (block_end - block_begin) / (ae_float_t)(zp_block_count - 1);

                    ae_uint32_t block_index = _index - property_index;

                    ae_float_t block_value = block_begin + block_add * (ae_float_t)block_index;

                    return block_value;
                }break;
            case 3:
                {
                    ae_uint32_t block_index = _index - property_index;

                    ae_float_t block_value = ((const ae_float_t *)(ae_constvoidptr_t)property_ae_uint32_t)[block_index];

                    return block_value;
                }break;
            }
        }
        else
        {
            switch( zp_block_type )
            {
            case 0:
                {
                    property_ae_uint32_t += 1;
                }break;
            case 1:
                {
                    property_ae_uint32_t += 2;
                }break;
            case 3:
                {
                    property_ae_uint32_t += zp_block_count;
                }break;
            }
        }

        property_index += zp_block_count;
    }

    return 0.f;
}
//////////////////////////////////////////////////////////////////////////
static ae_float_t __get_movie_transformation_property_interpolate( ae_constvoidptr_t _property, ae_uint32_t _index, ae_float_t _t )
{
    ae_float_t data_0 = __get_movie_transformation_property( _property, _index + 0 );
    ae_float_t data_1 = __get_movie_transformation_property( _property, _index + 1 );

    ae_float_t data = ae_linerp_f1( data_0, data_1, _t );

    return data;
}
//////////////////////////////////////////////////////////////////////////
#	define AE_INTERPOLATE_PROPERTY( Transformation, Name, OutName )\
	OutName = (Transformation->timeline == AE_NULL || Transformation->timeline->Name == AE_NULL) ? Transformation->immutable.Name : __get_movie_transformation_property_interpolate(\
		Transformation->timeline->Name,\
		_index, _t )
//////////////////////////////////////////////////////////////////////////
#	define AE_FIXED_PROPERTY( Transformation, Name, Index, OutName)\
	OutName = (Transformation->timeline == AE_NULL || Transformation->timeline->Name == AE_NULL) ? Transformation->immutable.Name : __get_movie_transformation_property(\
		Transformation->timeline->Name,\
		_index + Index )
//////////////////////////////////////////////////////////////////////////
static void __ae_movie_make_layer_transformation2d_immutable( ae_matrix4_t _out, const aeMovieLayerTransformation2D * _transformation )
{
    ae_vector2_t anchor_point;
    ae_vector2_t position;
    ae_vector2_t scale;
    ae_quaternion_t quaternion;

    ae_uint32_t _index = 0U;

    AE_FIXED_PROPERTY( _transformation, anchor_point_x, 0, anchor_point[0] );
    AE_FIXED_PROPERTY( _transformation, anchor_point_y, 0, anchor_point[1] );

    AE_FIXED_PROPERTY( _transformation, position_x, 0, position[0] );
    AE_FIXED_PROPERTY( _transformation, position_y, 0, position[1] );

    AE_FIXED_PROPERTY( _transformation, scale_x, 0, scale[0] );
    AE_FIXED_PROPERTY( _transformation, scale_y, 0, scale[1] );

    quaternion[0] = 0.f;
    quaternion[1] = 0.f;

    AE_FIXED_PROPERTY( _transformation, quaternion_z, 0, quaternion[2] );
    AE_FIXED_PROPERTY( _transformation, quaternion_w, 0, quaternion[3] );

    ae_movie_make_transformation2d_m4( _out, position, anchor_point, scale, quaternion );
}
//////////////////////////////////////////////////////////////////////////
static void __ae_movie_make_layer_transformation3d_immutable( ae_matrix4_t _out, const aeMovieLayerTransformation3D * _transformation )
{
    ae_vector3_t anchor_point;
    ae_vector3_t position;
    ae_vector3_t scale;
    ae_quaternion_t quaternion;

    ae_uint32_t _index = 0U;

    AE_FIXED_PROPERTY( _transformation, anchor_point_x, 0, anchor_point[0] );
    AE_FIXED_PROPERTY( _transformation, anchor_point_y, 0, anchor_point[1] );
    AE_FIXED_PROPERTY( _transformation, anchor_point_z, 0, anchor_point[2] );

    AE_FIXED_PROPERTY( _transformation, position_x, 0, position[0] );
    AE_FIXED_PROPERTY( _transformation, position_y, 0, position[1] );
    AE_FIXED_PROPERTY( _transformation, position_z, 0, position[2] );

    AE_FIXED_PROPERTY( _transformation, scale_x, 0, scale[0] );
    AE_FIXED_PROPERTY( _transformation, scale_y, 0, scale[1] );
    AE_FIXED_PROPERTY( _transformation, scale_z, 0, scale[2] );

    AE_FIXED_PROPERTY( _transformation, quaternion_x, 0, quaternion[0] );
    AE_FIXED_PROPERTY( _transformation, quaternion_y, 0, quaternion[1] );
    AE_FIXED_PROPERTY( _transformation, quaternion_z, 0, quaternion[2] );
    AE_FIXED_PROPERTY( _transformation, quaternion_w, 0, quaternion[3] );
    
    ae_movie_make_transformation3d_m4( _out, position, anchor_point, scale, quaternion );
}
//////////////////////////////////////////////////////////////////////////
#	define AE_MOVIE_STREAM_PROPERTY(Mask, Name)\
	if( _mask & Mask )\
	{\
		READ( _stream, _transformation->immutable.Name );\
		if( _transformation->timeline != AE_NULL ) {_transformation->timeline->Name = AE_NULL;}\
	}\
	else\
	{\
		_transformation->immutable.Name = 0.f;\
		_transformation->timeline->Name = __load_movie_layer_transformation_timeline(_stream, #Name);\
	}
//////////////////////////////////////////////////////////////////////////
static ae_result_t __ae_movie_load_layer_transformation2d( aeMovieStream * _stream, ae_uint32_t _mask, aeMovieLayerTransformation2D * _transformation )
{
    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_X, anchor_point_x );
    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Y, anchor_point_y );

    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_X, position_x );
    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_Y, position_y );

    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_X, scale_x );
    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_Y, scale_y );

    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_Z, quaternion_z );
    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_W, quaternion_w );

    if( (_transformation->immutable_property_mask & __AE_MOVIE_IMMUTABLE_TWO_D_ALL__) == __AE_MOVIE_IMMUTABLE_TWO_D_ALL__ )
    {
        __ae_movie_make_layer_transformation2d_immutable( _transformation->immutable_matrix, _transformation );
    }    

    return AE_MOVIE_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static ae_result_t __ae_movie_load_layer_transformation3d( aeMovieStream * _stream, ae_uint32_t _mask, aeMovieLayerTransformation3D * _transformation )
{
    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_X, anchor_point_x );
    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Y, anchor_point_y );
    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Z, anchor_point_z );

    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_X, position_x );
    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_Y, position_y );
    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_Z, position_z );

    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_X, scale_x );
    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_Y, scale_y );
    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_Z, scale_z );

    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_X, quaternion_x );
    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_Y, quaternion_y );
    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_Z, quaternion_z );
    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_W, quaternion_w );

    if( (_transformation->immutable_property_mask & __AE_MOVIE_IMMUTABLE_THREE_D_ALL__) == __AE_MOVIE_IMMUTABLE_THREE_D_ALL__ )
    {
        __ae_movie_make_layer_transformation3d_immutable( _transformation->immutable_matrix, _transformation );
    }

    return AE_MOVIE_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
#	undef AE_MOVIE_STREAM_PROPERTY
//////////////////////////////////////////////////////////////////////////
static void __movie_make_layer_transformation_interpolate_immutable( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index, ae_float_t _t )
{
    (void)_index;
    (void)_t;

    ae_copy_m4( _out, _transformation->immutable_matrix );
}
//////////////////////////////////////////////////////////////////////////
static void __movie_make_layer_transformation_fixed_immutable( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index )
{
    (void)_index;

    ae_copy_m4( _out, _transformation->immutable_matrix );
}
//////////////////////////////////////////////////////////////////////////
static void __movie_make_layer_transformation2d_fixed( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index )
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

    ae_movie_make_transformation2d_m4( _out, position, anchor_point, scale, quaternion );
}
//////////////////////////////////////////////////////////////////////////
static void __movie_make_layer_transformation2d_fixed_wq( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index )
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

    ae_movie_make_transformation2d_m4wq( _out, position, anchor_point, scale );
}
//////////////////////////////////////////////////////////////////////////
static void __movie_make_layer_transformation2d_interpolate_wq( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index, ae_float_t _t )
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

    ae_movie_make_transformation2d_m4wq( _out, position, anchor_point, scale );
}
//////////////////////////////////////////////////////////////////////////
static void __movie_make_layer_transformation2d_interpolate_fq( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index, ae_float_t _t )
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

    ae_movie_make_transformation2d_m4( _out, position, anchor_point, scale, quaternion );
}
//////////////////////////////////////////////////////////////////////////
static void __movie_make_layer_transformation2d_interpolate( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index, ae_float_t _t )
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

    ae_movie_make_transformation2d_m4( _out, position, anchor_point, scale, quaternion );
}
//////////////////////////////////////////////////////////////////////////
static void __movie_make_layer_transformation3d_fixed( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index )
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

    ae_movie_make_transformation3d_m4( _out, position, anchor_point, scale, quaternion );
}
//////////////////////////////////////////////////////////////////////////
static void __movie_make_layer_transformation3d_fixed_wq( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index )
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

    ae_movie_make_transformation3d_m4wq( _out, position, anchor_point, scale );
}
//////////////////////////////////////////////////////////////////////////
static void __movie_make_layer_transformation3d_interpolate_wq( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index, ae_float_t _t )
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

    ae_movie_make_transformation3d_m4wq( _out, position, anchor_point, scale );
}
//////////////////////////////////////////////////////////////////////////
static void __movie_make_layer_transformation3d_interpolate_fq( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index, ae_float_t _t )
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

    ae_movie_make_transformation3d_m4( _out, position, anchor_point, scale, quaternion );
}
//////////////////////////////////////////////////////////////////////////
static void __movie_make_layer_transformation3d_interpolate( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index, ae_float_t _t )
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

    ae_movie_make_transformation3d_m4( _out, position, anchor_point, scale, quaternion );
}
//////////////////////////////////////////////////////////////////////////
ae_result_t ae_movie_load_layer_transformation( aeMovieStream * _stream, aeMovieLayerTransformation * _transformation, ae_bool_t _threeD )
{
    ae_uint32_t immutable_property_mask;
    READ( _stream, immutable_property_mask );

    _transformation->immutable_property_mask = immutable_property_mask;

    if( immutable_property_mask & AE_MOVIE_IMMUTABLE_OPACITY )
    {
        READ( _stream, _transformation->immutable_opacity );
        _transformation->timeline_opacity = AE_NULL;
    }
    else
    {
        _transformation->immutable_opacity = 0.f;
        _transformation->timeline_opacity = __load_movie_layer_transformation_timeline( _stream, "immutable_opacity" );
    }

    if( _threeD == AE_FALSE )
    {
        aeMovieLayerTransformation2D * transformation2d = (aeMovieLayerTransformation2D *)_transformation;

        aeMovieLayerTransformation2DTimeline * timeline = AE_NULL;

        if( (immutable_property_mask & __AE_MOVIE_IMMUTABLE_TWO_D_ALL__) != __AE_MOVIE_IMMUTABLE_TWO_D_ALL__ )
        {
            timeline = AE_NEW( _stream->instance, aeMovieLayerTransformation2DTimeline );
        }
        
        transformation2d->timeline = timeline;

        ae_result_t result = __ae_movie_load_layer_transformation2d( _stream, immutable_property_mask, transformation2d );

        if( (immutable_property_mask & __AE_MOVIE_IMMUTABLE_TWO_D_ALL__) == __AE_MOVIE_IMMUTABLE_TWO_D_ALL__ )
        {
            transformation2d->transforamtion_interpolate_matrix = &__movie_make_layer_transformation_interpolate_immutable;
            transformation2d->transforamtion_fixed_matrix = &__movie_make_layer_transformation_fixed_immutable;
        }
        else
        {            
            if( (immutable_property_mask & __AE_MOVIE_IMMUTABLE_TWO_D_QUATERNION__) == __AE_MOVIE_IMMUTABLE_TWO_D_QUATERNION__ )
            {
                ae_uint32_t _index = 0U;

                ae_float_t q2;
                AE_FIXED_PROPERTY( transformation2d, quaternion_z, 0, q2 );

                ae_float_t q3;
                AE_FIXED_PROPERTY( transformation2d, quaternion_w, 0, q3 );

                if( q2 == 0.f && q3 == 1.f )
                {
                    transformation2d->transforamtion_interpolate_matrix = &__movie_make_layer_transformation2d_interpolate_wq;
                    transformation2d->transforamtion_fixed_matrix = &__movie_make_layer_transformation2d_fixed_wq;
                }
                else
                {
                    transformation2d->transforamtion_interpolate_matrix = &__movie_make_layer_transformation2d_interpolate_fq;
                    transformation2d->transforamtion_fixed_matrix = &__movie_make_layer_transformation2d_fixed;
                }
            }
            else
            {
                transformation2d->transforamtion_interpolate_matrix = &__movie_make_layer_transformation2d_interpolate;
                transformation2d->transforamtion_fixed_matrix = &__movie_make_layer_transformation2d_fixed;
            }
        }

        return result;
    }
    else
    {
        aeMovieLayerTransformation3D * transformation3d = (aeMovieLayerTransformation3D *)_transformation;

        aeMovieLayerTransformation3DTimeline * timeline = AE_NULL;

        if( (immutable_property_mask & __AE_MOVIE_IMMUTABLE_THREE_D_ALL__) != __AE_MOVIE_IMMUTABLE_THREE_D_ALL__ )
        {
            timeline = AE_NEW( _stream->instance, aeMovieLayerTransformation3DTimeline );
        }

        transformation3d->timeline = timeline;

        ae_result_t result = __ae_movie_load_layer_transformation3d( _stream, immutable_property_mask, transformation3d );

        if( (immutable_property_mask & __AE_MOVIE_IMMUTABLE_THREE_D_ALL__) == __AE_MOVIE_IMMUTABLE_THREE_D_ALL__ )
        {
            transformation3d->transforamtion_interpolate_matrix = &__movie_make_layer_transformation_interpolate_immutable;
            transformation3d->transforamtion_fixed_matrix = &__movie_make_layer_transformation_fixed_immutable;
        }
        else
        {
            if( (immutable_property_mask & __AE_MOVIE_IMMUTABLE_THREE_D_QUATERNION__) == __AE_MOVIE_IMMUTABLE_THREE_D_QUATERNION__ )
            {
                ae_uint32_t _index = 0U;

                ae_float_t q0;
                AE_FIXED_PROPERTY( transformation3d, quaternion_x, 0, q0 );

                ae_float_t q1;
                AE_FIXED_PROPERTY( transformation3d, quaternion_y, 0, q1 );

                ae_float_t q2;
                AE_FIXED_PROPERTY( transformation3d, quaternion_z, 0, q2 );

                ae_float_t q3;
                AE_FIXED_PROPERTY( transformation3d, quaternion_w, 0, q3 );

                if( q0 == 0.f && q1 == 0.f && q2 == 0.f && q3 == 1.f )
                {
                    transformation3d->transforamtion_interpolate_matrix = &__movie_make_layer_transformation3d_interpolate_wq;
                    transformation3d->transforamtion_fixed_matrix = &__movie_make_layer_transformation3d_fixed_wq;
                }
                else
                {
                    transformation3d->transforamtion_interpolate_matrix = &__movie_make_layer_transformation3d_interpolate_fq;
                    transformation3d->transforamtion_fixed_matrix = &__movie_make_layer_transformation3d_fixed;
                }
            }
            else
            {
                transformation3d->transforamtion_interpolate_matrix = &__movie_make_layer_transformation3d_interpolate;
                transformation3d->transforamtion_fixed_matrix = &__movie_make_layer_transformation3d_fixed;
            }
        }

        return result;
    }
}
//////////////////////////////////////////////////////////////////////////
#	define AE_MOVIE_STREAM_PROPERTY(Mask, Name)\
	if( _mask & Mask )\
	{\
		READ( _stream, _transformation->immutable.Name );\
		if( _transformation->timeline != AE_NULL ) {_transformation->timeline->Name = AE_NULL;}\
	}\
	else\
	{\
		_transformation->immutable.Name = 0.f;\
		_transformation->timeline->Name = __load_movie_layer_transformation_timeline(_stream, #Name);\
	}
//////////////////////////////////////////////////////////////////////////
static ae_result_t __ae_movie_load_camera_transformation( aeMovieStream * _stream, ae_uint32_t _mask, aeMovieCompositionCamera * _transformation )
{
    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_TARGET_X, target_x );
    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_TARGET_Y, target_y );
    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_TARGET_Z, target_z );

    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_X, position_x );
    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_Y, position_y );
    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_Z, position_z );

    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_X, quaternion_x );
    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_Y, quaternion_y );
    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_Z, quaternion_z );
    AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_W, quaternion_w );

    return AE_MOVIE_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
ae_result_t ae_movie_load_camera_transformation( aeMovieStream * _stream, aeMovieCompositionCamera * _camera )
{
    ae_uint32_t immutable_property_mask;
    READ( _stream, immutable_property_mask );

    _camera->immutable_property_mask = immutable_property_mask;

    aeMovieCompositionCameraTimeline * timeline = AE_NULL;

    if( (immutable_property_mask & __AE_MOVIE_IMMUTABLE_CAMERA_ALL__) != __AE_MOVIE_IMMUTABLE_CAMERA_ALL__ )
    {
        timeline = AE_NEW( _stream->instance, aeMovieCompositionCameraTimeline );
    }

    _camera->timeline = timeline;

    ae_result_t result = __ae_movie_load_camera_transformation( _stream, immutable_property_mask, _camera );

    return result;
}
//////////////////////////////////////////////////////////////////////////
static void __movie_delete_layer_transformation2d( const aeMovieInstance * _instance, const aeMovieLayerTransformation2D * _transformation )
{
    if( _transformation->timeline != AE_NULL )
    {
        aeMovieLayerTransformation2DTimeline * timeline = _transformation->timeline;

        AE_DELETEN( _instance, timeline->anchor_point_x );
        AE_DELETEN( _instance, timeline->anchor_point_y );
        AE_DELETEN( _instance, timeline->position_x );
        AE_DELETEN( _instance, timeline->position_y );
        AE_DELETEN( _instance, timeline->quaternion_z );
        AE_DELETEN( _instance, timeline->quaternion_w );
        AE_DELETEN( _instance, timeline->scale_x );
        AE_DELETEN( _instance, timeline->scale_y );

        AE_DELETE( _instance, _transformation->timeline );
    }
}
//////////////////////////////////////////////////////////////////////////
static void __movie_delete_layer_transformation3d( const aeMovieInstance * _instance, const aeMovieLayerTransformation3D * _transformation )
{
    if( _transformation->timeline != AE_NULL )
    {
        aeMovieLayerTransformation3DTimeline * timeline = _transformation->timeline;

        AE_DELETEN( _instance, timeline->anchor_point_x );
        AE_DELETEN( _instance, timeline->anchor_point_y );
        AE_DELETEN( _instance, timeline->anchor_point_z );
        AE_DELETEN( _instance, timeline->position_x );
        AE_DELETEN( _instance, timeline->position_y );
        AE_DELETEN( _instance, timeline->position_z );
        AE_DELETEN( _instance, timeline->quaternion_x );
        AE_DELETEN( _instance, timeline->quaternion_y );
        AE_DELETEN( _instance, timeline->quaternion_z );
        AE_DELETEN( _instance, timeline->quaternion_w );
        AE_DELETEN( _instance, timeline->scale_x );
        AE_DELETEN( _instance, timeline->scale_y );
        AE_DELETEN( _instance, timeline->scale_z );

        AE_DELETE( _instance, _transformation->timeline );
    }
}
//////////////////////////////////////////////////////////////////////////
void ae_movie_delete_layer_transformation( const aeMovieInstance * _instance, const aeMovieLayerTransformation * _transformation, ae_bool_t _threeD )
{
    AE_DELETEN( _instance, _transformation->timeline_opacity );

    if( _threeD == AE_FALSE )
    {
        __movie_delete_layer_transformation2d( _instance, (const aeMovieLayerTransformation2D *)_transformation );
    }
    else
    {
        __movie_delete_layer_transformation3d( _instance, (const aeMovieLayerTransformation3D *)_transformation );
    }
}
//////////////////////////////////////////////////////////////////////////
void ae_movie_make_layer_transformation_interpolate( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index, ae_float_t _t )
{
    (*_transformation->transforamtion_interpolate_matrix)(_out, _transformation, _index, _t);    
}
//////////////////////////////////////////////////////////////////////////
void ae_movie_make_layer_transformation_fixed( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_uint32_t _index )
{
    (*_transformation->transforamtion_fixed_matrix)(_out, _transformation, _index);
}
//////////////////////////////////////////////////////////////////////////
void ae_movie_make_camera_transformation( ae_vector3_t _target, ae_vector3_t _position, ae_quaternion_t _quaternion, const aeMovieCompositionCamera * _camera, ae_uint32_t _index, ae_bool_t _interpolate, ae_float_t _t )
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
ae_float_t ae_movie_make_layer_opacity( const aeMovieLayerTransformation * _transformation, ae_uint32_t _index, ae_bool_t _interpolate, ae_float_t _t )
{
    if( _transformation->timeline_opacity == AE_NULL )
    {
        return _transformation->immutable_opacity;
    }

    ae_float_t opacity;

    if( _interpolate == AE_TRUE )
    {
        opacity = __get_movie_transformation_property_interpolate( _transformation->timeline_opacity, _index, _t );
    }
    else
    {
        opacity = __get_movie_transformation_property( _transformation->timeline_opacity, _index );
    }

    return opacity;
}