#	ifndef MOVIE_MATRIX_H_
#	define MOVIE_MATRIX_H_

typedef float ae_vector3_t[3];
typedef float ae_vector4_t[4];
typedef float ae_matrix4_t[16];

void mul_v4_m4( ae_vector4_t _out, const ae_vector4_t _a, const ae_matrix4_t _b );
void mul_m4_m4( ae_matrix4_t _out, const ae_matrix4_t _a, const ae_matrix4_t _b );
void ident_m4( ae_matrix4_t _out );
void make_rotate_z_axis_m4( ae_matrix4_t _out, float _angle );
void make_rotate_m4_euler( ae_matrix4_t _out, float _x, float _y, float _z );
void make_transformation_m4( ae_matrix4_t _lm, const ae_vector3_t _position, const ae_vector3_t _origin, const ae_vector3_t _scale, const ae_vector3_t _orientation );

float angle_norm( float _angle );
float angle_correct_interpolate_from_to( float _from, float _to );

#	endif