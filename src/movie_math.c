#	include "movie_math.h"

#	include <math.h>

//////////////////////////////////////////////////////////////////////////
static const float f_eps = 0.00001f;
static const float f_neps = -0.00001f;
static const float pi = 3.14159265358979323846f;
static const float two_pi = 2.f * 3.14159265358979323846f;
static const float inv_two_pi = 1.f / (2.f * 3.14159265358979323846f);
static const float inv_255 = 1.f / 255.f;

static const float table_inv_255[] = {
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
	250.f / 255.f, 251.f / 255.f, 252.f / 255.f, 253.f / 255.f, 254.f / 255.f, 255.f / 255.f, 256.f / 255.f, 257.f / 255.f, 258.f / 255.f, 259.f / 255.f
};

//////////////////////////////////////////////////////////////////////////
#	define EQUAL_F_Z( v ) ((v >= f_neps) && (v <= f_eps))
#	define EQUAL_F_1( v ) ((v >= 1.f + f_neps) && (v <= 1.f + f_eps))
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
//////////////////////////////////////////////////////////////////////////
float minimax_f_f( float _v, float _min, float _max )
{
	return (_v > _min) ? ((_v < _max) ? _v : _max) : _min;
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
void make_quaternion_m4( ae_matrix4_t _m, const ae_quaternion_t _quaternion )
{
	float x = _quaternion[0];
	float y = _quaternion[1];
	float z = _quaternion[2];
	float w = _quaternion[3];

	float x2 = x + x;
	float y2 = y + y;
	float z2 = z + z;
	float xx = x * x2;
	float xy = x * y2;
	float xz = x * z2;
	float yy = y * y2;
	float yz = y * z2;
	float zz = z * z2;
	float wx = w * x2;
	float wy = w * y2;
	float wz = w * z2;

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
void make_transformation_m4( ae_matrix4_t _lm, const ae_vector3_t _position, const ae_vector3_t _origin, const ae_vector3_t _scale, const ae_quaternion_t _quaternion )
{
	if( EQUAL_F_Z( _quaternion[0] ) && 
		EQUAL_F_Z( _quaternion[1] ) &&
		EQUAL_F_Z( _quaternion[2] ) &&
		EQUAL_F_1( _quaternion[3] ) )
	{
		_lm[0 * 4 + 1] = 0.f;
		_lm[0 * 4 + 2] = 0.f;
		_lm[0 * 4 + 3] = 0.f;
		_lm[1 * 4 + 0] = 0.f;
		_lm[1 * 4 + 2] = 0.f;
		_lm[1 * 4 + 3] = 0.f;
		_lm[2 * 4 + 0] = 0.f;
		_lm[2 * 4 + 1] = 0.f;
		_lm[2 * 4 + 3] = 0.f;
		_lm[3 * 4 + 3] = 1.f;

		_lm[0 * 4 + 0] = _scale[0];
		_lm[1 * 4 + 1] = _scale[1];
		_lm[2 * 4 + 2] = _scale[2];

		_lm[3 * 4 + 0] = _position[0] - _origin[0] * _scale[0];
		_lm[3 * 4 + 1] = _position[1] - _origin[1] * _scale[1];
		_lm[3 * 4 + 2] = _position[2] - _origin[2] * _scale[2];
	}
	else
	{
		ae_matrix4_t mat_scale;
		
		mat_scale[0 * 4 + 1] = 0.f;
		mat_scale[0 * 4 + 2] = 0.f;
		mat_scale[0 * 4 + 3] = 0.f;
		mat_scale[1 * 4 + 0] = 0.f;
		mat_scale[1 * 4 + 2] = 0.f;
		mat_scale[1 * 4 + 3] = 0.f;
		mat_scale[2 * 4 + 0] = 0.f;
		mat_scale[2 * 4 + 1] = 0.f;
		mat_scale[2 * 4 + 3] = 0.f;
		mat_scale[3 * 4 + 3] = 1.f;

		mat_scale[0 * 4 + 0] = _scale[0];
		mat_scale[1 * 4 + 1] = _scale[1];
		mat_scale[2 * 4 + 2] = _scale[2];

		mat_scale[3 * 4 + 0] = -_origin[0] * _scale[0];
		mat_scale[3 * 4 + 1] = -_origin[1] * _scale[1];
		mat_scale[3 * 4 + 2] = -_origin[2] * _scale[2];

		ae_matrix4_t mat_rotate;
		make_quaternion_m4( mat_rotate, _quaternion );

		mul_m4_m4( _lm, mat_scale, mat_rotate );

		_lm[3 * 4 + 0] += _position[0];
		_lm[3 * 4 + 1] += _position[1];
		_lm[3 * 4 + 2] += _position[2];
	}
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
float linerp_f1( float _in1, float _in2, float _t )
{
	return _in1 * (1.f - _t) + _in2 * _t;
}
//////////////////////////////////////////////////////////////////////////
void linerp_f2( ae_vector2_t _out, const ae_vector2_t _in1, const ae_vector2_t _in2, float _t )
{
	_out[0] = linerp_f1( _in1[0], _in2[0], _t );
	_out[1] = linerp_f1( _in1[1], _in2[1], _t );
}
//////////////////////////////////////////////////////////////////////////
void linerp_q( ae_quaternion_t _q, const ae_quaternion_t _q1, const ae_quaternion_t _q2, float _t )
{
	float inv_t = 1.f - _t;

	_q[0] = _q1[0] * inv_t + _q2[0] * _t;
	_q[1] = _q1[1] * inv_t + _q2[1] * _t;
	_q[2] = _q1[2] * inv_t + _q2[2] * _t;
	_q[3] = _q1[3] * inv_t + _q2[3] * _t;
		
	float q_dot = _q[0] * _q[0] + _q[1] * _q[1] + _q[2] * _q[2] + _q[3] * _q[3];
	float inv_length = 1.f / sqrtf( q_dot );

	_q[0] *= inv_length;
	_q[1] *= inv_length;
	_q[2] *= inv_length;
	_q[3] *= inv_length;
}
//////////////////////////////////////////////////////////////////////////
float make_camera_fov( float _height, float _zoom )
{
	float camera_fov = atanf( (_height * 0.5f) / _zoom ) * 2.f;

	return camera_fov;
}
//////////////////////////////////////////////////////////////////////////
float linerp_c(const ae_color_t _c1, const ae_color_t _c2, float _t)
{
	if( _c1 == 255 && _c2 == 255 )
	{
		return 1.f;
	}

	float c1f = table_inv_255[_c1];
	float c2f = table_inv_255[_c2];

	float cf = linerp_f1(c1f, c2f, _t);

	return cf;
}
