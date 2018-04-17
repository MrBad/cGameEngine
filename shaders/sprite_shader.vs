// vertex shader
#version 130

// specify inputs

in vec2 vertexPosition;
in vec4 vertexColor;
in vec2 vertexUV;

out vec4 fragmentColor;
out vec2 fragmentPosition;
out vec2 fragmentUV;

uniform mat4 P;

void main()
{
	gl_Position.xy = (P * vec4(vertexPosition, 0.0, 1.0)).xy;
	gl_Position.z = 0.0;
	gl_Position.w = 1.0;

	fragmentColor = vertexColor;
	fragmentPosition = vertexPosition;
	fragmentUV = vertexUV;
}

