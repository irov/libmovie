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

#	include "movie_math.h"

//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_void_t __mul_v4_m4_r( ae_vector4_t _out, const ae_vector4_t _a, const ae_matrix4_t _b )
{
    _out[0] = _a[0] * _b[0 * 4 + 0] + _a[1] * _b[1 * 4 + 0] + _a[2] * _b[2 * 4 + 0];
    _out[1] = _a[0] * _b[0 * 4 + 1] + _a[1] * _b[1 * 4 + 1] + _a[2] * _b[2 * 4 + 1];
    _out[2] = _a[0] * _b[0 * 4 + 2] + _a[1] * _b[1 * 4 + 2] + _a[2] * _b[2 * 4 + 2];
    _out[3] = _a[3] * _b[3 * 4 + 3];
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_void_t __mul_v4_m4_rr( ae_vector4_t _out, const ae_vector4_t _a, const ae_matrix4_t _b )
{
    _out[0] = _a[0] * _b[0 * 4 + 0] + _a[1] * _b[1 * 4 + 0] + _a[2] * _b[2 * 4 + 0] + _a[3] * _b[3 * 4 + 0];
    _out[1] = _a[0] * _b[0 * 4 + 1] + _a[1] * _b[1 * 4 + 1] + _a[2] * _b[2 * 4 + 1] + _a[3] * _b[3 * 4 + 1];
    _out[2] = _a[0] * _b[0 * 4 + 2] + _a[1] * _b[1 * 4 + 2] + _a[2] * _b[2 * 4 + 2] + _a[3] * _b[3 * 4 + 2];
    _out[3] = _a[3] * _b[3 * 4 + 3];
}
//////////////////////////////////////////////////////////////////////////
ae_void_t ae_mul_m4_m4_r( ae_matrix4_t _out, const ae_matrix4_t _a, const ae_matrix4_t _b )
{
    __mul_v4_m4_r( _out + 0, _a + 0, _b );
    __mul_v4_m4_r( _out + 4, _a + 4, _b );
    __mul_v4_m4_r( _out + 8, _a + 8, _b );
    __mul_v4_m4_rr( _out + 12, _a + 12, _b );
}
//////////////////////////////////////////////////////////////////////////
ae_void_t ae_ident_m4( ae_matrix4_t _out )
{
    _out[0 * 4 + 0] = 1.f;
    _out[0 * 4 + 1] = 0.f;
    _out[0 * 4 + 2] = 0.f;
    _out[0 * 4 + 3] = 0.f;

    _out[1 * 4 + 0] = 0.f;
    _out[1 * 4 + 1] = 1.f;
    _out[1 * 4 + 2] = 0.f;
    _out[1 * 4 + 3] = 0.f;

    _out[2 * 4 + 0] = 0.f;
    _out[2 * 4 + 1] = 0.f;
    _out[2 * 4 + 2] = 1.f;
    _out[2 * 4 + 3] = 0.f;

    _out[3 * 4 + 0] = 0.f;
    _out[3 * 4 + 1] = 0.f;
    _out[3 * 4 + 2] = 0.f;
    _out[3 * 4 + 3] = 1.f;
}
//////////////////////////////////////////////////////////////////////////
ae_void_t ae_copy_m4( ae_matrix4_t _out, const ae_matrix4_t _in )
{
    *_out++ = *_in++;
    *_out++ = *_in++;
    *_out++ = *_in++;
    *_out++ = *_in++;

    *_out++ = *_in++;
    *_out++ = *_in++;
    *_out++ = *_in++;
    *_out++ = *_in++;

    *_out++ = *_in++;
    *_out++ = *_in++;
    *_out++ = *_in++;
    *_out++ = *_in++;

    *_out++ = *_in++;
    *_out++ = *_in++;
    *_out++ = *_in++;
    *_out++ = *_in++;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_void_t __make_quaternion_m4( ae_matrix4_t _m, const ae_quaternion_t _quaternion )
{
    ae_float_t x = _quaternion[0];
    ae_float_t y = _quaternion[1];
    ae_float_t z = _quaternion[2];
    ae_float_t w = _quaternion[3];

    ae_float_t x2 = x + x;
    ae_float_t y2 = y + y;
    ae_float_t z2 = z + z;
    ae_float_t xx = x * x2;
    ae_float_t xy = x * y2;
    ae_float_t xz = x * z2;
    ae_float_t yy = y * y2;
    ae_float_t yz = y * z2;
    ae_float_t zz = z * z2;
    ae_float_t wx = w * x2;
    ae_float_t wy = w * y2;
    ae_float_t wz = w * z2;

    _m[0 * 4 + 0] = 1.f - (yy + zz);
    _m[0 * 4 + 1] = xy - wz;
    _m[0 * 4 + 2] = xz + wy;
    _m[0 * 4 + 3] = 0.f;

    _m[1 * 4 + 0] = xy + wz;
    _m[1 * 4 + 1] = 1.f - (xx + zz);
    _m[1 * 4 + 2] = yz - wx;
    _m[1 * 4 + 3] = 0.f;

    _m[2 * 4 + 0] = xz - wy;
    _m[2 * 4 + 1] = yz + wx;
    _m[2 * 4 + 2] = 1.f - (xx + yy);
    _m[2 * 4 + 3] = 0.f;

    _m[3 * 4 + 0] = 0.f;
    _m[3 * 4 + 1] = 0.f;
    _m[3 * 4 + 2] = 0.f;
    _m[3 * 4 + 3] = 1.0f;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_void_t __make_quaternionzw_m4( ae_matrix4_t _m, const ae_quaternionzw_t _quaternion )
{
    ae_float_t z = _quaternion[0];
    ae_float_t w = _quaternion[1];

    ae_float_t z2 = z + z;
    ae_float_t zz = z * z2;
    ae_float_t wz = w * z2;

    _m[0 * 4 + 0] = 1.f - zz;
    _m[0 * 4 + 1] = -wz;
    _m[0 * 4 + 2] = 0.f;
    _m[0 * 4 + 3] = 0.f;

    _m[1 * 4 + 0] = +wz;
    _m[1 * 4 + 1] = 1.f - zz;
    _m[1 * 4 + 2] = 0.f;
    _m[1 * 4 + 3] = 0.f;

    _m[2 * 4 + 0] = 0.f;
    _m[2 * 4 + 1] = 0.f;
    _m[2 * 4 + 2] = 1.f;
    _m[2 * 4 + 3] = 0.f;

    _m[3 * 4 + 0] = 0.f;
    _m[3 * 4 + 1] = 0.f;
    _m[3 * 4 + 2] = 0.f;
    _m[3 * 4 + 3] = 1.0f;
}
//////////////////////////////////////////////////////////////////////////
ae_void_t ae_movie_make_transformation3d_m4wq( ae_matrix4_t _out, const ae_vector3_t _position, const ae_vector3_t _origin, const ae_vector3_t _scale )
{
    _out[0 * 4 + 0] = _scale[0];
    _out[0 * 4 + 1] = 0.f;
    _out[0 * 4 + 2] = 0.f;
    _out[0 * 4 + 3] = 0.f;

    _out[1 * 4 + 0] = 0.f;
    _out[1 * 4 + 1] = _scale[1];
    _out[1 * 4 + 2] = 0.f;
    _out[1 * 4 + 3] = 0.f;

    _out[2 * 4 + 0] = 0.f;
    _out[2 * 4 + 1] = 0.f;
    _out[2 * 4 + 2] = _scale[2];
    _out[2 * 4 + 3] = 0.f;

    _out[3 * 4 + 0] = _position[0] - _origin[0] * _scale[0];
    _out[3 * 4 + 1] = _position[1] - _origin[1] * _scale[1];
    _out[3 * 4 + 2] = _position[2] - _origin[2] * _scale[2];
    _out[3 * 4 + 3] = 1.f;
}
//////////////////////////////////////////////////////////////////////////
ae_void_t ae_movie_make_transformation3d_m4( ae_matrix4_t _out, const ae_vector3_t _position, const ae_vector3_t _anchor, const ae_vector3_t _scale, const ae_quaternion_t _quaternion )
{
    ae_matrix4_t mat_scale;

    mat_scale[0 * 4 + 0] = _scale[0];
    mat_scale[0 * 4 + 1] = 0.f;
    mat_scale[0 * 4 + 2] = 0.f;
    mat_scale[0 * 4 + 3] = 0.f;

    mat_scale[1 * 4 + 0] = 0.f;
    mat_scale[1 * 4 + 1] = _scale[1];
    mat_scale[1 * 4 + 2] = 0.f;
    mat_scale[1 * 4 + 3] = 0.f;

    mat_scale[2 * 4 + 0] = 0.f;
    mat_scale[2 * 4 + 1] = 0.f;
    mat_scale[2 * 4 + 2] = _scale[2];
    mat_scale[2 * 4 + 3] = 0.f;

    mat_scale[3 * 4 + 0] = -_anchor[0] * _scale[0];
    mat_scale[3 * 4 + 1] = -_anchor[1] * _scale[1];
    mat_scale[3 * 4 + 2] = -_anchor[2] * _scale[2];
    mat_scale[3 * 4 + 3] = 1.f;

    ae_matrix4_t mat_rotate;
    __make_quaternion_m4( mat_rotate, _quaternion );

    ae_mul_m4_m4_r( _out, mat_scale, mat_rotate );

    _out[3 * 4 + 0] += _position[0];
    _out[3 * 4 + 1] += _position[1];
    _out[3 * 4 + 2] += _position[2];
}
//////////////////////////////////////////////////////////////////////////
ae_void_t ae_movie_make_transformation2d_m4wq( ae_matrix4_t _out, const ae_vector2_t _position, const ae_vector2_t _anchor, const ae_vector2_t _scale )
{
    _out[0 * 4 + 0] = _scale[0];
    _out[0 * 4 + 1] = 0.f;
    _out[0 * 4 + 2] = 0.f;
    _out[0 * 4 + 3] = 0.f;

    _out[1 * 4 + 0] = 0.f;
    _out[1 * 4 + 1] = _scale[1];
    _out[1 * 4 + 2] = 0.f;
    _out[1 * 4 + 3] = 0.f;

    _out[2 * 4 + 0] = 0.f;
    _out[2 * 4 + 1] = 0.f;
    _out[2 * 4 + 2] = 1.f;
    _out[2 * 4 + 3] = 0.f;

    _out[3 * 4 + 0] = _position[0] - _anchor[0] * _scale[0];
    _out[3 * 4 + 1] = _position[1] - _anchor[1] * _scale[1];
    _out[3 * 4 + 2] = 0.f;
    _out[3 * 4 + 3] = 1.f;
}
//////////////////////////////////////////////////////////////////////////
ae_void_t ae_movie_make_transformation2d_m4( ae_matrix4_t _out, const ae_vector2_t _position, const ae_vector2_t _anchor, const ae_vector2_t _scale, const ae_quaternionzw_t _quaternion )
{
    ae_matrix4_t mat_base;

    mat_base[0 * 4 + 0] = _scale[0];
    mat_base[0 * 4 + 1] = 0.f;
    mat_base[0 * 4 + 2] = 0.f;
    mat_base[0 * 4 + 3] = 0.f;

    mat_base[1 * 4 + 0] = 0.f;
    mat_base[1 * 4 + 1] = _scale[1];
    mat_base[1 * 4 + 2] = 0.f;
    mat_base[1 * 4 + 3] = 0.f;

    mat_base[2 * 4 + 0] = 0.f;
    mat_base[2 * 4 + 1] = 0.f;
    mat_base[2 * 4 + 2] = 1.f;
    mat_base[2 * 4 + 3] = 0.f;

    mat_base[3 * 4 + 0] = -_anchor[0] * _scale[0];
    mat_base[3 * 4 + 1] = -_anchor[1] * _scale[1];
    mat_base[3 * 4 + 2] = 0.f;
    mat_base[3 * 4 + 3] = 1.f;

    ae_matrix4_t mat_rotate;
    __make_quaternionzw_m4( mat_rotate, _quaternion );

    ae_mul_m4_m4_r( _out, mat_base, mat_rotate );

    _out[3 * 4 + 0] += _position[0];
    _out[3 * 4 + 1] += _position[1];
}
//////////////////////////////////////////////////////////////////////////
union __inverse_sqrtf_alias_cast_t
{
    ae_float_t raw;
    ae_uint32_t data;
};
//////////////////////////////////////////////////////////////////////////
static const ae_float_t __inverse_sqrtf_threehalfs = 1.5f;
static const ae_uint32_t __inverse_sqrtf_magic_number = 0x5F3759DF;
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_float_t __inverse_sqrtf( ae_float_t _number )
{
    ae_float_t x2 = _number * 0.5f;

    union __inverse_sqrtf_alias_cast_t i_cast;
    i_cast.raw = _number;
    ae_uint32_t i = i_cast.data;
    
    i = __inverse_sqrtf_magic_number - (i >> 1);

    union __inverse_sqrtf_alias_cast_t f_cast;
    f_cast.data = i;

    ae_float_t y = f_cast.raw;

    y = y * (__inverse_sqrtf_threehalfs - (x2 * y * y));
    y = y * (__inverse_sqrtf_threehalfs - (x2 * y * y));

    return y;
}
//////////////////////////////////////////////////////////////////////////
ae_void_t ae_linerp_q( ae_quaternion_t _q, const ae_quaternion_t _q1, const ae_quaternion_t _q2, ae_float_t _t )
{
    ae_float_t inv_t = 1.f - _t;

    ae_float_t x = _q1[0] * inv_t + _q2[0] * _t;
    ae_float_t y = _q1[1] * inv_t + _q2[1] * _t;
    ae_float_t z = _q1[2] * inv_t + _q2[2] * _t;
    ae_float_t w = _q1[3] * inv_t + _q2[3] * _t;

    ae_float_t q_dot = x * x + y * y + z * z + w * w;
    ae_float_t inv_length = __inverse_sqrtf( q_dot );

    _q[0] = x * inv_length;
    _q[1] = y * inv_length;
    _q[2] = z * inv_length;
    _q[3] = w * inv_length;
}
//////////////////////////////////////////////////////////////////////////
ae_void_t ae_linerp_qzw( ae_quaternionzw_t _q, const ae_quaternionzw_t _q1, const ae_quaternionzw_t _q2, ae_float_t _t )
{
    ae_float_t inv_t = 1.f - _t;

    ae_float_t z = _q1[0] * inv_t + _q2[0] * _t;
    ae_float_t w = _q1[1] * inv_t + _q2[1] * _t;

    ae_float_t q_dot = z * z + w * w;
    ae_float_t inv_length = __inverse_sqrtf( q_dot );

    _q[0] = z * inv_length;
    _q[1] = w * inv_length;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_void_t __line_from_two_point_v2( ae_float_t * _line, ae_float_t _ax, ae_float_t _ay, ae_float_t _bx, ae_float_t _by )
{
    _line[0] = _ay - _by;
    _line[1] = _bx - _ax;
    _line[2] = _ax * _by - _bx * _ay;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_float_t __line_dot( ae_float_t _a, ae_float_t _b, ae_float_t _c, ae_float_t _d )
{
    ae_float_t dot = _a * _d - _c * _b;

    return dot;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_void_t __line_intersect_v2_np( ae_float_t * _out, const ae_float_t * _l1, const ae_float_t * _l2 )
{
    ae_float_t zn = __line_dot( _l1[0], _l1[1], _l2[0], _l2[1] );

    _out[0] = -__line_dot( _l1[2], _l1[1], _l2[2], _l2[1] ) / zn;
    _out[1] = -__line_dot( _l1[0], _l1[2], _l2[0], _l2[2] ) / zn;
}
//////////////////////////////////////////////////////////////////////////
ae_void_t ae_multiply_tetragon_uv4_v2( ae_float_t * _out, const ae_vector2_t * _uv, const ae_float_t * _p )
{
    const ae_float_t * A = _uv[0];
    const ae_float_t * B = _uv[1];
    const ae_float_t * C = _uv[2];
    const ae_float_t * D = _uv[3];

    const ae_float_t ABx = A[0] + (B[0] - A[0]) * _p[0];
    const ae_float_t ABy = A[1] + (B[1] - A[1]) * _p[0];
    const ae_float_t DCx = D[0] + (C[0] - D[0]) * _p[0];
    const ae_float_t DCy = D[1] + (C[1] - D[1]) * _p[0];

    const ae_float_t ADx = A[0] + (D[0] - A[0]) * _p[1];
    const ae_float_t ADy = A[1] + (D[1] - A[1]) * _p[1];
    const ae_float_t BCx = B[0] + (C[0] - B[0]) * _p[1];
    const ae_float_t BCy = B[1] + (C[1] - B[1]) * _p[1];

    ae_float_t uv_l1[3];
    __line_from_two_point_v2( uv_l1, ABx, ABy, DCx, DCy );

    ae_float_t uv_l2[3];
    __line_from_two_point_v2( uv_l2, ADx, ADy, BCx, BCy );

    __line_intersect_v2_np( _out, uv_l1, uv_l2 );
}
