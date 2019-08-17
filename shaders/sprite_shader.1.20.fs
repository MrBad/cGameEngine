#version 120

// ->
varying vec2 fragmentPosition;
varying vec4 fragmentColor;
varying vec2 fragmentUV;

// uniform float time;
uniform sampler2D mySampler;

void main()
{

	vec4 textureColor = texture2D(mySampler, vec2(fragmentUV.x, -fragmentUV.y));
	gl_FragColor = fragmentColor * textureColor;
}
