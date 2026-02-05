#version 450 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D texture_y;
uniform sampler2D texture_cr;
uniform sampler2D texture_cb;

mat4 rec601 = mat4(
	1.16438,  0.00000,  1.59603, -0.87079,
	1.16438, -0.39176, -0.81297,  0.52959,
	1.16438,  2.01723,  0.00000, -1.08139,
	0, 0, 0, 1
);

void main() {
	float y = texture2D(texture_y, TexCoord).r;
	float cb = texture2D(texture_cb, TexCoord).r;
	float cr = texture2D(texture_cr, TexCoord).r;

	FragColor = vec4(y, cb, cr, 1.0) * rec601;
	// FragColor = vec4(TexCoord.x, TexCoord.y, 1, 1);
}
