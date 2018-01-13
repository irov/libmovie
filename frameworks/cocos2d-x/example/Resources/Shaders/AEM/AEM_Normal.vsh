//
// TODO: rework together with AEM_TrackMatte to a single program using #define TRACK_MATTE
//

attribute vec4 a_position;
attribute vec2 a_texCoord;
attribute vec4 a_color;

#ifdef GL_ES
varying lowp vec4 v_fragmentColor;
varying mediump vec2 v_texCoord;
#else
varying vec4 v_fragmentColor;
varying vec2 v_texCoord;
#endif

void main()
{
	gl_Position = CC_PMatrix * a_position;
	v_fragmentColor = a_color;
	v_texCoord = a_texCoord;
}
