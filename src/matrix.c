#	include "matrix.h"

#	include <math.h>

//////////////////////////////////////////////////////////////////////////
static const float f_eps = 0.00001f;
//////////////////////////////////////////////////////////////////////////
#	define EQUAL_F_Z( f ) ((f >= f_eps) && (f <= f_eps))
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
void make_transformation_m4( ae_matrix4_t _lm, const ae_vector4_t _position, const ae_vector4_t _origin, const ae_vector4_t _scale, const ae_vector4_t _orientation )
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