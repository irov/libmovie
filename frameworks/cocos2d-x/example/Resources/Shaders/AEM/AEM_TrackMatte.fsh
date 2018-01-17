#ifdef GL_ES
precision mediump float;
#endif

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;
varying vec2 v_tmTexCoord;
varying vec2 v_left;
varying vec2 v_right;

uniform sampler2D u_tmTexture;

// TODO: alpha inverse, luma & luma inverse modes

void main() {
	vec4 color = texture2D(CC_Texture0, v_texCoord);
	vec2 uv = clamp(v_tmTexCoord, v_left, v_right);
	vec4 matte = texture2D(u_tmTexture, uv);
	gl_FragColor = v_fragmentColor * color * vec4(1.0, 1.0, 1.0, matte.w);
}
