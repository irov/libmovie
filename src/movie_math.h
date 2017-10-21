/******************************************************************************
* libMOVIE Software License v1.0
*
* Copyright (c) 2016-2017, Yuriy Levchenko <irov13@mail.ru>
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

ae_bool_t ae_equal_f_z( ae_float_t _a );
ae_bool_t ae_equal_f_f( ae_float_t _a, ae_float_t _b );

ae_float_t ae_min_f_f( ae_float_t _a, ae_float_t _b );
ae_float_t ae_max_f_f( ae_float_t _a, ae_float_t _b );
ae_float_t ae_minimax_f_f( ae_float_t _v, ae_float_t _min, ae_float_t _max );

void ae_mul_v3_v2_m4( ae_vector3_t _out, const ae_vector2_t _a, const ae_matrix4_t _b );
void ae_mul_m4_m4_r( ae_matrix4_t _out, const ae_matrix4_t _a, const ae_matrix4_t _b );
void ae_ident_m4( ae_matrix4_t _out );
void ae_copy_m4( ae_matrix4_t _out, const ae_matrix4_t _in );
void ae_movie_make_transformation3d_m4( ae_matrix4_t _out, const ae_vector3_t _position, const ae_vector3_t _anchor, const ae_vector3_t _scale, const ae_quaternion_t _quaternion );
void ae_movie_make_transformation3d_m4wq( ae_matrix4_t _out, const ae_vector3_t _position, const ae_vector3_t _anchor, const ae_vector3_t _scale );
void ae_movie_make_transformation2d_m4( ae_matrix4_t _out, const ae_vector2_t _position, const ae_vector2_t _anchor, const ae_vector2_t _scale, const ae_quaternionzw_t _quaternion );
void ae_movie_make_transformation2d_m4wq( ae_matrix4_t _out, const ae_vector2_t _position, const ae_vector2_t _anchor, const ae_vector2_t _scale );

ae_float_t ae_linerp_f1( ae_float_t _in1, ae_float_t _in2, ae_float_t _t );
void ae_linerp_f2( ae_vector2_t _out, const ae_vector2_t _in1, const ae_vector2_t _in2, ae_float_t _t );
void ae_linerp_q( ae_quaternion_t _q, const ae_quaternion_t _q1, const ae_quaternion_t _q2, ae_float_t _t );
void ae_linerp_qzw( ae_quaternionzw_t _q, const ae_quaternionzw_t _q1, const ae_quaternionzw_t _q2, ae_float_t _t );
ae_float_t ae_linerp_c( const ae_color_t _c1, const ae_color_t _c2, ae_float_t _t );

ae_float_t ae_tof_c( const ae_color_t _c );

void ae_multiply_tetragon_uv4_v2( ae_float_t * _out, const ae_vector2_t * _uv, const ae_float_t * _p );

#	endif