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

#	ifndef MOVIE_MATRIX_H_
#	define MOVIE_MATRIX_H_

#	include "movie/movie_type.h"

//////////////////////////////////////////////////////////////////////////
static const ae_float_t f_eps = 0.00001f;
static const ae_float_t f_neps = -0.00001f;
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_bool_t ae_equal_f_z( ae_float_t _a )
{
    return ((_a >= 0.f + f_neps) && (_a <= 0.f + f_eps)) ? AE_TRUE : AE_FALSE;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_bool_t ae_equal_f_f( ae_float_t _a, ae_float_t _b )
{
    return ((_a >= _b + f_neps) && (_a <= _b + f_eps)) ? AE_TRUE : AE_FALSE;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_float_t ae_min_f_f( ae_float_t _a, ae_float_t _b )
{
    return (_a > _b) ? _b : _a;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_float_t ae_max_f_f( ae_float_t _a, ae_float_t _b )
{
    return (_a > _b) ? _a : _b;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_float_t ae_minimax_f_f( ae_float_t _v, ae_float_t _min, ae_float_t _max )
{
    return (_v > _min) ? ((_v < _max) ? _v : _max) : _min;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_void_t ae_mul_v3_v2_m4( ae_vector3_t _out, const ae_vector2_t _a, const ae_matrix4_t _b )
{
    _out[0] = _a[0] * _b[0 * 4 + 0] + _a[1] * _b[1 * 4 + 0] + _b[3 * 4 + 0];
    _out[1] = _a[0] * _b[0 * 4 + 1] + _a[1] * _b[1 * 4 + 1] + _b[3 * 4 + 1];
    _out[2] = _a[0] * _b[0 * 4 + 2] + _a[1] * _b[1 * 4 + 2] + _b[3 * 4 + 2];
}
//////////////////////////////////////////////////////////////////////////
ae_void_t ae_mul_m4_m4_r( ae_matrix4_t _out, const ae_matrix4_t _a, const ae_matrix4_t _b );
ae_void_t ae_ident_m4( ae_matrix4_t _out );
ae_void_t ae_copy_m4( ae_matrix4_t _out, const ae_matrix4_t _in );
ae_void_t ae_movie_make_transformation3d_m4( ae_matrix4_t _out, const ae_vector3_t _position, const ae_vector3_t _anchor, const ae_vector3_t _scale, const ae_quaternion_t _quaternion );
ae_void_t ae_movie_make_transformation3d_m4wq( ae_matrix4_t _out, const ae_vector3_t _position, const ae_vector3_t _anchor, const ae_vector3_t _scale );
ae_void_t ae_movie_make_transformation2d_m4( ae_matrix4_t _out, const ae_vector2_t _position, const ae_vector2_t _anchor, const ae_vector2_t _scale, const ae_quaternionzw_t _quaternion );
ae_void_t ae_movie_make_transformation2d_m4wq( ae_matrix4_t _out, const ae_vector2_t _position, const ae_vector2_t _anchor, const ae_vector2_t _scale );
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_float_t ae_linerp_f1( ae_float_t _in1, ae_float_t _in2, ae_float_t _t )
{
    return _in1 * (1.f - _t) + _in2 * _t;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_void_t ae_linerp_f2( ae_vector2_t _out, const ae_vector2_t _in1, const ae_vector2_t _in2, ae_float_t _t )
{
    _out[0] = ae_linerp_f1( _in1[0], _in2[0], _t );
    _out[1] = ae_linerp_f1( _in1[1], _in2[1], _t );
}
//////////////////////////////////////////////////////////////////////////
ae_void_t ae_linerp_q( ae_quaternion_t _q, const ae_quaternion_t _q1, const ae_quaternion_t _q2, ae_float_t _t );
ae_void_t ae_linerp_qzw( ae_quaternionzw_t _q, const ae_quaternionzw_t _q1, const ae_quaternionzw_t _q2, ae_float_t _t );
//////////////////////////////////////////////////////////////////////////
static const ae_float_t table_inv_255[] = {
    0.f / 255.f, 1.f / 255.f, 2.f / 255.f, 3.f / 255.f, 4.f / 255.f, 5.f / 255.f, 6.f / 255.f, 7.f / 255.f, 8.f / 255.f, 9.f / 255.f,
    10.f / 255.f, 11.f / 255.f, 12.f / 255.f, 13.f / 255.f, 14.f / 255.f, 15.f / 255.f, 16.f / 255.f, 17.f / 255.f, 18.f / 255.f, 19.f / 255.f,
    20.f / 255.f, 21.f / 255.f, 22.f / 255.f, 23.f / 255.f, 24.f / 255.f, 25.f / 255.f, 26.f / 255.f, 27.f / 255.f, 28.f / 255.f, 29.f / 255.f,
    30.f / 255.f, 31.f / 255.f, 32.f / 255.f, 33.f / 255.f, 34.f / 255.f, 35.f / 255.f, 36.f / 255.f, 37.f / 255.f, 38.f / 255.f, 39.f / 255.f,
    40.f / 255.f, 41.f / 255.f, 42.f / 255.f, 43.f / 255.f, 44.f / 255.f, 45.f / 255.f, 46.f / 255.f, 47.f / 255.f, 48.f / 255.f, 49.f / 255.f,
    50.f / 255.f, 51.f / 255.f, 52.f / 255.f, 53.f / 255.f, 54.f / 255.f, 55.f / 255.f, 56.f / 255.f, 57.f / 255.f, 58.f / 255.f, 59.f / 255.f,
    60.f / 255.f, 61.f / 255.f, 62.f / 255.f, 63.f / 255.f, 64.f / 255.f, 65.f / 255.f, 66.f / 255.f, 67.f / 255.f, 68.f / 255.f, 69.f / 255.f,
    70.f / 255.f, 71.f / 255.f, 72.f / 255.f, 73.f / 255.f, 74.f / 255.f, 75.f / 255.f, 76.f / 255.f, 77.f / 255.f, 78.f / 255.f, 79.f / 255.f,
    80.f / 255.f, 81.f / 255.f, 82.f / 255.f, 83.f / 255.f, 84.f / 255.f, 85.f / 255.f, 86.f / 255.f, 87.f / 255.f, 88.f / 255.f, 89.f / 255.f,
    90.f / 255.f, 91.f / 255.f, 92.f / 255.f, 93.f / 255.f, 94.f / 255.f, 95.f / 255.f, 96.f / 255.f, 97.f / 255.f, 98.f / 255.f, 99.f / 255.f,
    100.f / 255.f, 101.f / 255.f, 102.f / 255.f, 103.f / 255.f, 104.f / 255.f, 105.f / 255.f, 106.f / 255.f, 107.f / 255.f, 108.f / 255.f, 109.f / 255.f,
    110.f / 255.f, 111.f / 255.f, 112.f / 255.f, 113.f / 255.f, 114.f / 255.f, 115.f / 255.f, 116.f / 255.f, 117.f / 255.f, 118.f / 255.f, 119.f / 255.f,
    120.f / 255.f, 121.f / 255.f, 122.f / 255.f, 123.f / 255.f, 124.f / 255.f, 125.f / 255.f, 126.f / 255.f, 127.f / 255.f, 128.f / 255.f, 129.f / 255.f,
    130.f / 255.f, 131.f / 255.f, 132.f / 255.f, 133.f / 255.f, 134.f / 255.f, 135.f / 255.f, 136.f / 255.f, 137.f / 255.f, 138.f / 255.f, 139.f / 255.f,
    140.f / 255.f, 141.f / 255.f, 142.f / 255.f, 143.f / 255.f, 144.f / 255.f, 145.f / 255.f, 146.f / 255.f, 147.f / 255.f, 148.f / 255.f, 149.f / 255.f,
    150.f / 255.f, 151.f / 255.f, 152.f / 255.f, 153.f / 255.f, 154.f / 255.f, 155.f / 255.f, 156.f / 255.f, 157.f / 255.f, 158.f / 255.f, 159.f / 255.f,
    160.f / 255.f, 161.f / 255.f, 162.f / 255.f, 163.f / 255.f, 164.f / 255.f, 165.f / 255.f, 166.f / 255.f, 167.f / 255.f, 168.f / 255.f, 169.f / 255.f,
    170.f / 255.f, 171.f / 255.f, 172.f / 255.f, 173.f / 255.f, 174.f / 255.f, 175.f / 255.f, 176.f / 255.f, 177.f / 255.f, 178.f / 255.f, 179.f / 255.f,
    180.f / 255.f, 181.f / 255.f, 182.f / 255.f, 183.f / 255.f, 184.f / 255.f, 185.f / 255.f, 186.f / 255.f, 187.f / 255.f, 188.f / 255.f, 189.f / 255.f,
    190.f / 255.f, 191.f / 255.f, 192.f / 255.f, 193.f / 255.f, 194.f / 255.f, 195.f / 255.f, 196.f / 255.f, 197.f / 255.f, 198.f / 255.f, 199.f / 255.f,
    200.f / 255.f, 201.f / 255.f, 202.f / 255.f, 203.f / 255.f, 204.f / 255.f, 205.f / 255.f, 206.f / 255.f, 207.f / 255.f, 208.f / 255.f, 209.f / 255.f,
    210.f / 255.f, 211.f / 255.f, 212.f / 255.f, 213.f / 255.f, 214.f / 255.f, 215.f / 255.f, 216.f / 255.f, 217.f / 255.f, 218.f / 255.f, 219.f / 255.f,
    220.f / 255.f, 221.f / 255.f, 222.f / 255.f, 223.f / 255.f, 224.f / 255.f, 225.f / 255.f, 226.f / 255.f, 227.f / 255.f, 228.f / 255.f, 229.f / 255.f,
    230.f / 255.f, 231.f / 255.f, 232.f / 255.f, 233.f / 255.f, 234.f / 255.f, 235.f / 255.f, 236.f / 255.f, 237.f / 255.f, 238.f / 255.f, 239.f / 255.f,
    240.f / 255.f, 241.f / 255.f, 242.f / 255.f, 243.f / 255.f, 244.f / 255.f, 245.f / 255.f, 246.f / 255.f, 247.f / 255.f, 248.f / 255.f, 249.f / 255.f,
    250.f / 255.f, 251.f / 255.f, 252.f / 255.f, 253.f / 255.f, 254.f / 255.f, 255.f / 255.f
};
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_float_t ae_linerp_c( const ae_color_t _c1, const ae_color_t _c2, ae_float_t _t )
{
    ae_float_t c1f = table_inv_255[_c1];
    ae_float_t c2f = table_inv_255[_c2];

    ae_float_t cf = ae_linerp_f1( c1f, c2f, _t );

    return cf;
}
//////////////////////////////////////////////////////////////////////////
AE_INTERNAL ae_float_t ae_tof_c( const ae_color_t _c )
{
    ae_float_t cf = table_inv_255[_c];

    return cf;
}
//////////////////////////////////////////////////////////////////////////
ae_void_t ae_multiply_tetragon_uv4_v2( ae_float_t * _out, const ae_vector2_t * _uv, const ae_float_t * _p );

#	endif