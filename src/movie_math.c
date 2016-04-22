#	include "movie_math.h"

#	include <math.h>

//////////////////////////////////////////////////////////////////////////
static const float f_eps = 0.00001f;
static const float f_neps = -0.00001f;
static const float pi = 3.14159265358979323846f;
static const float two_pi = 2.f * 3.14159265358979323846f;
static const float inv_two_pi = 1.f / (2.f * 3.14159265358979323846f);
//////////////////////////////////////////////////////////////////////////
#	define EQUAL_F_Z( f ) ((f >= f_neps) && (f <= f_eps))
//////////////////////////////////////////////////////////////////////////
ae_bool_t equal_f_z( float _a )
{
	return (_a >= 0.f + f_neps) && (_a <= 0.f + f_eps) ? AE_TRUE : AE_FALSE;
}
//////////////////////////////////////////////////////////////////////////
ae_bool_t equal_f_f( float _a, float _b )
{
	return (_a >= _b + f_neps) && (_a <= _b + f_eps) ? AE_TRUE : AE_FALSE;
}
//////////////////////////////////////////////////////////////////////////
float min_f_f( float _a, float _b )
{
	return (_a > _b) ? _b : _a;
}
//////////////////////////////////////////////////////////////////////////
float max_f_f( float _a, float _b )
{
	return (_a > _b) ? _a : _b;
}
void mul_v2_v2_m4( ae_vector2_t _out, const ae_vector2_t _a, const ae_matrix4_t _b )
{
	_out[0] = _a[0] * _b[0 * 4 + 0] + _a[1] * _b[1 * 4 + 0] + _b[3 * 4 + 0];
	_out[1] = _a[0] * _b[0 * 4 + 1] + _a[1] * _b[1 * 4 + 1] + _b[3 * 4 + 1];
}
//////////////////////////////////////////////////////////////////////////
void mul_v3_v2_m4( ae_vector3_t _out, const ae_vector2_t _a, const ae_matrix4_t _b )
{
	_out[0] = _a[0] * _b[0 * 4 + 0] + _a[1] * _b[1 * 4 + 0] + _b[3 * 4 + 0];
	_out[1] = _a[0] * _b[0 * 4 + 1] + _a[1] * _b[1 * 4 + 1] + _b[3 * 4 + 1];
	_out[2] = _a[0] * _b[0 * 4 + 2] + _a[1] * _b[1 * 4 + 2] + _b[3 * 4 + 2];
}
//////////////////////////////////////////////////////////////////////////
void mul_v4_m4( ae_vector4_t _out, const ae_vector4_t _a, const ae_matrix4_t _b )
{
	_out[0] = _a[0] * _b[0 * 4 + 0] + _a[1] * _b[1 * 4 + 0] + _a[2] * _b[2 * 4 + 0] + _a[3] * _b[3 * 4 + 0];
	_out[1] = _a[0] * _b[0 * 4 + 1] + _a[1] * _b[1 * 4 + 1] + _a[2] * _b[2 * 4 + 1] + _a[3] * _b[3 * 4 + 1];
	_out[2] = _a[0] * _b[0 * 4 + 2] + _a[1] * _b[1 * 4 + 2] + _a[2] * _b[2 * 4 + 2] + _a[3] * _b[3 * 4 + 2];
	_out[3] = _a[0] * _b[0 * 4 + 3] + _a[1] * _b[1 * 4 + 3] + _a[2] * _b[2 * 4 + 3] + _a[3] * _b[3 * 4 + 3];
}
//////////////////////////////////////////////////////////////////////////
void mul_m4_m4( ae_matrix4_t _out, const ae_matrix4_t _a, const ae_matrix4_t _b )
{
	mul_v4_m4( _out + 0, _a + 0, _b );
	mul_v4_m4( _out + 4, _a + 4, _b );
	mul_v4_m4( _out + 8, _a + 8, _b );
	mul_v4_m4( _out + 12, _a + 12, _b );
}
//////////////////////////////////////////////////////////////////////////
void mul_v3_v3_m4_homogenize( ae_vector3_t _out, const ae_vector3_t _a, const ae_matrix4_t _b )
{
	_out[0] = _a[0] * _b[0 * 4 + 0] + _a[1] * _b[1 * 4 + 0] + _a[2] * _b[2 * 4 + 0] + _b[3 * 4 + 0];
	_out[1] = _a[0] * _b[0 * 4 + 1] + _a[1] * _b[1 * 4 + 1] + _a[2] * _b[2 * 4 + 1] + _b[3 * 4 + 1];
	_out[2] = _a[0] * _b[0 * 4 + 2] + _a[1] * _b[1 * 4 + 2] + _a[2] * _b[2 * 4 + 2] + _b[3 * 4 + 2];
	float w = _a[0] * _b[0 * 4 + 3] + _a[1] * _b[1 * 4 + 3] + _a[2] * _b[2 * 4 + 3] + _b[3 * 4 + 3];

	float w_inv = 1.f / w;

	_out[0] *= w_inv;
	_out[1] *= w_inv;
	_out[2] *= w_inv;
}
//////////////////////////////////////////////////////////////////////////
void ident_m4( ae_matrix4_t _out )
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
void copy_m4( ae_matrix4_t _out, const ae_matrix4_t _in )
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
void make_rotate_z_axis_m4( float * _out, float _angle )
{
	float c = cosf( _angle );
	float s = sinf( _angle );

	_out[0 * 4 + 0] = c;
	_out[0 * 4 + 1] = -s;
	_out[0 * 4 + 2] = 0.f;
	_out[0 * 4 + 3] = 0.f;

	_out[1 * 4 + 0] = s;
	_out[1 * 4 + 1] = c;
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
void make_rotate_m4_euler( ae_matrix4_t _out, float _x, float _y, float _z )
{
	float ca = cosf( _x );
	float cb = cosf( _y );
	float cy = cosf( _z );

	float sa = sinf( _x );
	float sb = sinf( _y );
	float sy = sinf( _z );

	_out[0 * 4 + 0] = ca * cb;
	_out[0 * 4 + 1] = ca * sb * sy - sa * cy;
	_out[0 * 4 + 2] = ca * sb * cy + sa * sy;
	_out[0 * 4 + 3] = 0.f;

	_out[1 * 4 + 0] = sa * cb;
	_out[1 * 4 + 1] = sa * sb * sy + ca * cy;
	_out[1 * 4 + 2] = sa * sb * cy - ca * sy;
	_out[1 * 4 + 3] = 0.f;

	_out[2 * 4 + 0] = -sb;
	_out[2 * 4 + 1] = cb * sy;
	_out[2 * 4 + 2] = cb * cy;
	_out[2 * 4 + 3] = 0.f;

	_out[3 * 4 + 0] = 0.f;
	_out[3 * 4 + 1] = 0.f;
	_out[3 * 4 + 2] = 0.f;
	_out[3 * 4 + 3] = 1.f;
}
//////////////////////////////////////////////////////////////////////////
void make_transformation_m4( ae_matrix4_t _lm, const ae_vector3_t _position, const ae_vector3_t _origin, const ae_vector3_t _scale, const ae_vector3_t _orientation )
{
	ae_matrix4_t mat_scale;
	ident_m4( mat_scale );

	mat_scale[3 * 4 + 0] = -_origin[0] * _scale[0];
	mat_scale[3 * 4 + 1] = -_origin[1] * _scale[1];
	mat_scale[3 * 4 + 2] = -_origin[2] * _scale[2];

	mat_scale[0 * 4 + 0] = _scale[0];
	mat_scale[1 * 4 + 1] = _scale[1];
	mat_scale[2 * 4 + 2] = _scale[2];

	if( EQUAL_F_Z( _orientation[1] ) &&
		EQUAL_F_Z( _orientation[2] ) )
	{
		if( EQUAL_F_Z( _orientation[0] ) )
		{
			copy_m4( _lm, mat_scale );
		}
		else
		{
			ae_matrix4_t mat_rot;
			make_rotate_z_axis_m4( mat_rot, _orientation[0] );

			mul_m4_m4( _lm, mat_scale, mat_rot );
		}
	}
	else
	{
		ae_matrix4_t mat_rot;
		make_rotate_m4_euler( mat_rot, _orientation[0], _orientation[1], _orientation[2] );

		mul_m4_m4( _lm, mat_scale, mat_rot );
	}

	_lm[3 * 4 + 0] += _position[0];
	_lm[3 * 4 + 1] += _position[1];
	_lm[3 * 4 + 2] += _position[2];
}
//////////////////////////////////////////////////////////////////////////
void norm_v3_v3( ae_vector3_t _out, const ae_vector3_t _rhs )
{
	float invL = 1.f / sqrtf( _rhs[0] * _rhs[0] + _rhs[1] * _rhs[1] + _rhs[2] * _rhs[2] );

	_out[0] = _rhs[0] * invL;
	_out[1] = _rhs[1] * invL;
	_out[2] = _rhs[2] * invL;
}
//////////////////////////////////////////////////////////////////////////
void cross_v3_v3( ae_vector3_t _out, const ae_vector3_t _a, const ae_vector3_t _b )
{
	_out[0] = _a[1] * _b[2] - _a[2] * _b[1];
	_out[1] = _a[2] * _b[0] - _a[0] * _b[2];
	_out[2] = _a[0] * _b[1] - _a[1] * _b[0];
}
//////////////////////////////////////////////////////////////////////////
void cross_v3_v3_norm( ae_vector3_t _out, const ae_vector3_t _a, const ae_vector3_t _b )
{
	cross_v3_v3( _out, _a, _b );
	norm_v3_v3( _out, _out );
}
//////////////////////////////////////////////////////////////////////////
float dot_v3_v3( const ae_vector3_t a, const ae_vector3_t b )
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}
//////////////////////////////////////////////////////////////////////////
void make_lookat_m4( ae_matrix4_t _out, const ae_vector3_t _eye, const ae_vector3_t _dir, const ae_vector3_t _up )
{
	ae_vector3_t zaxis;
	norm_v3_v3( zaxis, _dir );

	ae_vector3_t xaxis;
	cross_v3_v3_norm( xaxis, _up, zaxis );

	ae_vector3_t yaxis;
	cross_v3_v3( yaxis, zaxis, xaxis );

	_out[0 * 4 + 0] = xaxis[0];
	_out[0 * 4 + 1] = yaxis[0];
	_out[0 * 4 + 2] = zaxis[0];
	_out[0 * 4 + 3] = 0.f;

	_out[1 * 4 + 0] = xaxis[1];
	_out[1 * 4 + 1] = yaxis[1];
	_out[1 * 4 + 2] = zaxis[1];
	_out[1 * 4 + 3] = 0.f;

	_out[2 * 4 + 0] = xaxis[2];
	_out[2 * 4 + 1] = yaxis[2];
	_out[2 * 4 + 2] = zaxis[2];
	_out[2 * 4 + 3] = 0.f;

	_out[3 * 4 + 0] = -dot_v3_v3( xaxis, _eye );
	_out[3 * 4 + 1] = -dot_v3_v3( yaxis, _eye );
	_out[3 * 4 + 2] = -dot_v3_v3( zaxis, _eye );
	_out[3 * 4 + 3] = 1.f;
}
//////////////////////////////////////////////////////////////////////////
void make_projection_fov_m4( ae_matrix4_t _out, float fovy, float aspect, float zn, float zf )
{
	float yscale = 1.f / tanf( fovy * 0.5f );
	float xscale = yscale / aspect;

	_out[0 * 4 + 0] = xscale;
	_out[0 * 4 + 1] = 0.f;
	_out[0 * 4 + 2] = 0.f;
	_out[0 * 4 + 3] = 0.f;

	_out[1 * 4 + 0] = 0.f;
	_out[1 * 4 + 1] = yscale;
	_out[1 * 4 + 2] = 0.f;
	_out[1 * 4 + 3] = 0.f;

	_out[2 * 4 + 0] = 0.f;
	_out[2 * 4 + 1] = 0.f;
	_out[2 * 4 + 2] = zf / (zf - zn);
	_out[2 * 4 + 3] = 1.f;

	_out[3 * 4 + 0] = 0.f;
	_out[3 * 4 + 1] = 0.f;
	_out[3 * 4 + 2] = -zn * zf / (zf - zn);
	_out[3 * 4 + 3] = 0.f;
}
//////////////////////////////////////////////////////////////////////////
float angle_norm( float _angle )
{
	if( _angle < 0.f )
	{
		float pi_count = floorf( _angle * inv_two_pi );
		float pi_abs = pi_count * two_pi;

		_angle -= pi_abs;
	}

	if( _angle > two_pi )
	{
		float pi_count = floorf( _angle * inv_two_pi );
		float pi_abs = pi_count * two_pi;

		_angle -= pi_abs;
	}

	return _angle;
}
//////////////////////////////////////////////////////////////////////////
float angle_correct_interpolate_from_to( float _from, float _to )
{	
	float norm_angle_to = angle_norm( _to );

	float correct_angle = norm_angle_to;

	if( norm_angle_to > _from )
	{
		float dist = norm_angle_to - _from;

		if( (_from + two_pi) - norm_angle_to < dist )
		{
			correct_angle = norm_angle_to - two_pi;
		}
	}
	else
	{
		float dist = _from - norm_angle_to;

		if( (norm_angle_to + two_pi) - _from < dist )
		{
			correct_angle = norm_angle_to + two_pi;
		}
	}

	return correct_angle;
}
//////////////////////////////////////////////////////////////////////////
float linerp_f1( float _in1, float _in2, float _scale )
{
	return _in1 + (_in2 - _in1) * _scale;
}
//////////////////////////////////////////////////////////////////////////
float make_camera_fov( float _height, float _zoom )
{
	float camera_fov = atanf( (_height * 0.5f) / _zoom ) * 2.f;

	return camera_fov;
}