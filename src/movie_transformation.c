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
static const void * __load_movie_layer_transformation_timeline( aeMovieStream * _stream, const char * _doc )
{
	(void)_doc;

	uint32_t zp_data_size;
	READ( _stream, zp_data_size );
				
#	ifndef _DEBUG
	if( zp_data_size % 4 != 0 )
	{
		return AE_NULL;
	}
#	endif

	uint32_t hashmask_iterator;
	READ( _stream, hashmask_iterator );

	void * timeline = NEWV( _stream->instance, _doc, zp_data_size );
	READV( _stream, timeline, (size_t)zp_data_size );

	const uint32_t * hashmask = _stream->instance->hashmask;

	for( uint32_t 
		*it = (uint32_t *)timeline,
		*it_end = (uint32_t *)timeline + zp_data_size / 4;
		it != it_end;
		++it )
	{
		uint32_t hashmask_index = hashmask_iterator++ % 5;

		uint32_t hash = hashmask[hashmask_index];

		*it ^= hash;
	}

	return timeline;
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
static ae_result_t __ae_movie_load_layer_transformation2d( aeMovieStream * _stream, uint32_t _mask, aeMovieLayerTransformation2D * _transformation )
{
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_X, anchor_point_x );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_ANCHOR_POINT_Y, anchor_point_y );

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_X, position_x );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_POSITION_Y, position_y );

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_X, scale_x );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_SCALE_Y, scale_y );

	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_Z, quaternion_z );
	AE_MOVIE_STREAM_PROPERTY( AE_MOVIE_IMMUTABLE_QUATERNION_W, quaternion_w );
	
	return AE_MOVIE_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
static ae_result_t __ae_movie_load_layer_transformation3d( aeMovieStream * _stream, uint32_t _mask, aeMovieLayerTransformation3D * _transformation )
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

	return AE_MOVIE_SUCCESSFUL;
}
//////////////////////////////////////////////////////////////////////////
#	undef AE_MOVIE_STREAM_PROPERTY
//////////////////////////////////////////////////////////////////////////
ae_result_t ae_movie_load_layer_transformation( aeMovieStream * _stream, aeMovieLayerTransformation * _transformation, ae_bool_t _threeD )
{
	uint32_t immutable_property_mask;
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
		aeMovieLayerTransformation2DTimeline * timeline = AE_NULL;

		if( (immutable_property_mask & __AE_MOVIE_IMMUTABLE_TWO_D_ALL__) != __AE_MOVIE_IMMUTABLE_TWO_D_ALL__ )
        {
            timeline = NEW( _stream->instance, aeMovieLayerTransformation2DTimeline );
        }

		((aeMovieLayerTransformation2D *)_transformation)->timeline = timeline;

        ae_result_t result = __ae_movie_load_layer_transformation2d( _stream, immutable_property_mask, (aeMovieLayerTransformation2D *)_transformation );

        return result;
    }
    else
    {
		aeMovieLayerTransformation3DTimeline * timeline = AE_NULL;

		if( (immutable_property_mask & __AE_MOVIE_IMMUTABLE_THREE_D_ALL__) != __AE_MOVIE_IMMUTABLE_THREE_D_ALL__ )
        {
            timeline = NEW( _stream->instance, aeMovieLayerTransformation3DTimeline );
        }

		((aeMovieLayerTransformation3D *)_transformation)->timeline = timeline;

        ae_result_t result = __ae_movie_load_layer_transformation3d( _stream, immutable_property_mask, (aeMovieLayerTransformation3D *)_transformation );

        return result;
    }
}
//////////////////////////////////////////////////////////////////////////
static void __ae_movie_delete_layer_transformation2d( const aeMovieInstance * _instance, const aeMovieLayerTransformation2D * _transformation )
{
	if( _transformation->timeline != AE_NULL )
	{
		aeMovieLayerTransformation2DTimeline * timeline = _transformation->timeline;

		DELETEN( _instance, timeline->anchor_point_x );
		DELETEN( _instance, timeline->anchor_point_y );
		DELETEN( _instance, timeline->position_x );
		DELETEN( _instance, timeline->position_y );
		DELETEN( _instance, timeline->quaternion_z );
		DELETEN( _instance, timeline->quaternion_w );
		DELETEN( _instance, timeline->scale_x );
		DELETEN( _instance, timeline->scale_y );

		DELETE( _instance, _transformation->timeline );
	}
}
//////////////////////////////////////////////////////////////////////////
static void __ae_movie_delete_layer_transformation3d( const aeMovieInstance * _instance, const aeMovieLayerTransformation3D * _transformation )
{
	if( _transformation->timeline != AE_NULL )
	{
		aeMovieLayerTransformation3DTimeline * timeline = _transformation->timeline;

		DELETEN( _instance, timeline->anchor_point_x );
		DELETEN( _instance, timeline->anchor_point_y );
		DELETEN( _instance, timeline->anchor_point_z );
		DELETEN( _instance, timeline->position_x );
		DELETEN( _instance, timeline->position_y );
		DELETEN( _instance, timeline->position_z );
		DELETEN( _instance, timeline->quaternion_x );
		DELETEN( _instance, timeline->quaternion_y );
		DELETEN( _instance, timeline->quaternion_z );
		DELETEN( _instance, timeline->quaternion_w );
		DELETEN( _instance, timeline->scale_x );
		DELETEN( _instance, timeline->scale_y );
		DELETEN( _instance, timeline->scale_z );

		DELETE( _instance, _transformation->timeline );
	}
}
//////////////////////////////////////////////////////////////////////////
void ae_movie_delete_layer_transformation( const aeMovieInstance * _instance, const aeMovieLayerTransformation * _transformation, ae_bool_t _threeD )
{
	DELETEN( _instance, _transformation->timeline_opacity );

	if( _threeD == AE_FALSE )
	{
		__ae_movie_delete_layer_transformation2d( _instance, (const aeMovieLayerTransformation2D *)_transformation );
	}
	else
	{
        __ae_movie_delete_layer_transformation3d( _instance, (const aeMovieLayerTransformation3D *)_transformation );
	}
}
//////////////////////////////////////////////////////////////////////////
static float __get_movie_layer_transformation_property( const void * _property, uint32_t _index )
{
    uint32_t property_index = 0;

    const uint32_t * property_uint32_t = (const uint32_t *)_property;

    uint32_t zp_count = *(property_uint32_t++);

    uint32_t i = 0;
    for( ; i != zp_count; ++i )
    {
        uint32_t zp_block_type_count_data = *(property_uint32_t++);

        uint32_t zp_block_type = zp_block_type_count_data >> 24;
        uint32_t zp_block_count = zp_block_type_count_data & 0x00FFFFFF;

        if( property_index + zp_block_count > _index )
        {
            switch( zp_block_type )
            {
            case 0:
                {
                    float block_value = *(const float *)(const void *)(property_uint32_t);

                    return block_value;
                }break;
            case 1:
                {
                    float block_begin = *(const float *)(const void *)(property_uint32_t++);
                    float block_end = *(const float *)(const void *)(property_uint32_t);

                    float block_add = (block_end - block_begin) / (float)(zp_block_count - 1);

                    uint32_t block_index = _index - property_index;

                    float block_value = block_begin + block_add * (float)block_index;

                    return block_value;
                }break;
            case 3:
                {
                    uint32_t block_index = _index - property_index;

                    float block_value = ((const float *)(const void *)property_uint32_t)[block_index];

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
                    property_uint32_t += 1;
                }break;
            case 1:
                {
                    property_uint32_t += 2;
                }break;
            case 3:
                {
                    property_uint32_t += zp_block_count;
                }break;
            }
        }

        property_index += zp_block_count;
    }

    return 0.f;
}
//////////////////////////////////////////////////////////////////////////
static float __get_movie_layer_transformation_property_interpolate( const void * _property, uint32_t _index, float _t )
{
	float data_0 = __get_movie_layer_transformation_property( _property, _index + 0 );
	float data_1 = __get_movie_layer_transformation_property( _property, _index + 1 );

	float data = linerp_f1( data_0, data_1, _t );

	return data;
}
//////////////////////////////////////////////////////////////////////////
#	define AE_INTERPOLATE_PROPERTY( Name, OutName )\
	OutName = (_transformation->timeline == AE_NULL || _transformation->timeline->Name == AE_NULL) ? _transformation->immutable.Name : __get_movie_layer_transformation_property_interpolate(\
		_transformation->timeline->Name,\
		_index, _t )
//////////////////////////////////////////////////////////////////////////
#	define AE_FIXED_PROPERTY( Name, Index, OutName)\
	OutName = (_transformation->timeline == AE_NULL || _transformation->timeline->Name == AE_NULL) ? _transformation->immutable.Name : __get_movie_layer_transformation_property(\
		_transformation->timeline->Name,\
		_index + Index )
//////////////////////////////////////////////////////////////////////////
static void __ae_movie_make_layer_transformation2d( ae_matrix4_t _out, const aeMovieLayerTransformation2D * _transformation, uint32_t _index, ae_bool_t _interpolate, float _t )
{
	ae_vector2_t anchor_point;
	ae_vector2_t position;
	ae_vector2_t scale;
	ae_quaternion_t quaternion;

	if( _interpolate == AE_TRUE )
	{
		AE_INTERPOLATE_PROPERTY( anchor_point_x, anchor_point[0] );
		AE_INTERPOLATE_PROPERTY( anchor_point_y, anchor_point[1] );

		AE_INTERPOLATE_PROPERTY( position_x, position[0] );
		AE_INTERPOLATE_PROPERTY( position_y, position[1] );

		AE_INTERPOLATE_PROPERTY( scale_x, scale[0] );
		AE_INTERPOLATE_PROPERTY( scale_y, scale[1] );

		ae_quaternion_t q1;
		q1[0] = 0.f;
		q1[1] = 0.f;
		AE_FIXED_PROPERTY( quaternion_z, 0, q1[2] );
		AE_FIXED_PROPERTY( quaternion_w, 0, q1[3] );

		ae_quaternion_t q2;
		q2[0] = 0.f;
		q2[1] = 0.f;
		AE_FIXED_PROPERTY( quaternion_z, 1, q2[2] );
		AE_FIXED_PROPERTY( quaternion_w, 1, q2[3] );

		linerp_qzw( quaternion, q1, q2, _t );
	}
	else
	{
		AE_FIXED_PROPERTY( anchor_point_x, 0, anchor_point[0] );
		AE_FIXED_PROPERTY( anchor_point_y, 0, anchor_point[1] );

		AE_FIXED_PROPERTY( position_x, 0, position[0] );
		AE_FIXED_PROPERTY( position_y, 0, position[1] );

		AE_FIXED_PROPERTY( scale_x, 0, scale[0] );
		AE_FIXED_PROPERTY( scale_y, 0, scale[1] );

		quaternion[0] = 0.f;
		quaternion[1] = 0.f;

		AE_FIXED_PROPERTY( quaternion_z, 0, quaternion[2] );
		AE_FIXED_PROPERTY( quaternion_w, 0, quaternion[3] );
	}

	ae_movie_make_transformation2d_m4( _out, position, anchor_point, scale, quaternion );
}
//////////////////////////////////////////////////////////////////////////
static void __ae_movie_make_layer_transformation3d( ae_matrix4_t _out, const aeMovieLayerTransformation3D * _transformation, uint32_t _index, ae_bool_t _interpolate, float _t )
{
	ae_vector3_t anchor_point;
	ae_vector3_t position;
	ae_vector3_t scale;
	ae_quaternion_t quaternion;

	if( _interpolate == AE_TRUE )
	{
		AE_INTERPOLATE_PROPERTY( anchor_point_x, anchor_point[0] );
		AE_INTERPOLATE_PROPERTY( anchor_point_y, anchor_point[1] );
		AE_INTERPOLATE_PROPERTY( anchor_point_z, anchor_point[2] );

		AE_INTERPOLATE_PROPERTY( position_x, position[0] );
		AE_INTERPOLATE_PROPERTY( position_y, position[1] );
		AE_INTERPOLATE_PROPERTY( position_z, position[2] );

		AE_INTERPOLATE_PROPERTY( scale_x, scale[0] );
		AE_INTERPOLATE_PROPERTY( scale_y, scale[1] );
		AE_INTERPOLATE_PROPERTY( scale_z, scale[2] );

		ae_quaternion_t q1;
		AE_FIXED_PROPERTY( quaternion_x, 0, q1[0] );
		AE_FIXED_PROPERTY( quaternion_y, 0, q1[1] );
		AE_FIXED_PROPERTY( quaternion_z, 0, q1[2] );
		AE_FIXED_PROPERTY( quaternion_w, 0, q1[3] );

		ae_quaternion_t q2;
		AE_FIXED_PROPERTY( quaternion_x, 1, q2[0] );
		AE_FIXED_PROPERTY( quaternion_y, 1, q2[1] );
		AE_FIXED_PROPERTY( quaternion_z, 1, q2[2] );
		AE_FIXED_PROPERTY( quaternion_w, 1, q2[3] );

		linerp_q( quaternion, q1, q2, _t );
	}
	else
	{
		AE_FIXED_PROPERTY( anchor_point_x, 0, anchor_point[0] );
		AE_FIXED_PROPERTY( anchor_point_y, 0, anchor_point[1] );
		AE_FIXED_PROPERTY( anchor_point_z, 0, anchor_point[2] );

		AE_FIXED_PROPERTY( position_x, 0, position[0] );
		AE_FIXED_PROPERTY( position_y, 0, position[1] );
		AE_FIXED_PROPERTY( position_z, 0, position[2] );

		AE_FIXED_PROPERTY( scale_x, 0, scale[0] );
		AE_FIXED_PROPERTY( scale_y, 0, scale[1] );
		AE_FIXED_PROPERTY( scale_z, 0, scale[2] );

		AE_FIXED_PROPERTY( quaternion_x, 0, quaternion[0] );
		AE_FIXED_PROPERTY( quaternion_y, 0, quaternion[1] );
		AE_FIXED_PROPERTY( quaternion_z, 0, quaternion[2] );
		AE_FIXED_PROPERTY( quaternion_w, 0, quaternion[3] );
	}

	ae_movie_make_transformation3d_m4( _out, position, anchor_point, scale, quaternion );
}
//////////////////////////////////////////////////////////////////////////
#	undef AE_INTERPOLATE_PROPERTY
#	undef AE_FIXED_PROPERTY
//////////////////////////////////////////////////////////////////////////
void ae_movie_make_layer_transformation( ae_matrix4_t _out, const aeMovieLayerTransformation * _transformation, ae_bool_t _threeD, uint32_t _index, ae_bool_t _interpolate, float _t )
{
	if( _threeD == AE_FALSE )
	{ 
		__ae_movie_make_layer_transformation2d( _out, (const aeMovieLayerTransformation2D *)_transformation, _index, _interpolate, _t );
	}
	else
	{
		__ae_movie_make_layer_transformation3d( _out, (const aeMovieLayerTransformation3D *)_transformation, _index, _interpolate, _t );
	}
}
//////////////////////////////////////////////////////////////////////////
float ae_movie_make_layer_opacity( const aeMovieLayerTransformation * _transformation, uint32_t _index, ae_bool_t _interpolate, float _t )
{
	if( _transformation->timeline_opacity == AE_NULL )
	{
		return _transformation->immutable_opacity;
	}

	float opacity;
		
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