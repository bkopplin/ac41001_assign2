// Minimal fragment shader
// for AC41001 Lab 5
// You should modify this fragment shader to apply texture
// appropriately

#version 420

in vec4 fcolour;
in vec2 ftexcoord;
in vec4 fposition;
out vec4 outputColor;

layout (binding=0) uniform sampler2D tex1;

float fog_maxdist = 8.0;
float fog_mindist = 0.1;
vec4 fog_colour = vec4(0.4, 0.4, 0.4, 1.0);
const float fogDensity = 0.6f;
uint fogmode = 2;
float fog_factor = 0;

void main()
{
	float dist = length(fposition.xyz);
	//float fog_factor = (fog_maxdist - dist) / (fog_maxdist - fog_mindist);
	if (fogmode == 1) 
		fog_factor = (fog_maxdist - dist) / (fog_maxdist - fog_mindist);
	else if (fogmode == 2)
		fog_factor = 1.0 / exp(dist * fogDensity);
	else if (fogmode == 3)
		fog_factor = 1.0 / pow(exp(dist * fogDensity), 2);

	fog_factor = clamp(fog_factor, 0.0, 1.0);

	vec4 texcolour = texture(tex1, ftexcoord);
	outputColor = fcolour * texcolour;
	//outputColor = mix(fog_colour, fcolour * texcolour, fog_factor) ;
}