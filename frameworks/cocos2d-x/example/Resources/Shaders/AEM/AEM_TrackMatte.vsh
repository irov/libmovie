attribute vec4 a_position;
attribute vec4 a_color;
attribute vec2 a_texCoord;

uniform vec3 u_clip0;
uniform vec3 u_clip1;
uniform vec3 u_clip2;
uniform vec3 u_clip3;

uniform vec2 left;
uniform vec2 right;

#ifdef GL_ES
varying lowp vec4 v_fragmentColor;
varying mediump vec2 v_texCoord;
varying mediump vec2 v_tmTexCoord;
varying mediump vec2 v_left;
varying mediump vec2 v_right;
#else
varying vec4 v_fragmentColor;
varying vec2 v_texCoord;
varying vec2 v_tmTexCoord;
varying vec2 v_left;
varying vec2 v_right;
#endif

void main()
{
	gl_Position = CC_PMatrix * a_position;
	v_fragmentColor = a_color;
	v_texCoord = a_texCoord;
	v_left = left;
	v_right = right;

	float a = dot(u_clip0, vec3(1.0, a_position.x, a_position.y));
	float b = dot(u_clip1, vec3(1.0, a_position.x, a_position.y));
	v_tmTexCoord.x = dot(u_clip2, vec3(1.0, a, b));
	v_tmTexCoord.y = dot(u_clip3, vec3(1.0, a, b));
}
