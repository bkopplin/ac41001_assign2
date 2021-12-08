/*
AC41001 Graphics Assignment 2 
Bjarne Kopplin 2021
*/


#version 400

// inputs
in vec4 fcolour;
in vec4 fposition;

// outputs
out vec4 outputColor;

// diffuse and specular lighting
in vec3 fambient;
in vec3 N, L, R, V;
in float dL; // distance to light source
vec3 specular_colour = vec3(1.0, 0.8, 0.6);
float shininess = 12.f;

// fog
float fog_maxdist = 90.0;
float fog_mindist = 22.1;
vec4 fog_colour = vec4(0.2, 0.2, 0.2, 1.0);
const float fogDensity = 0.05f;
uint fogmode = 1;
float fog_factor = 0;


void main()
{
vec4 diffuse_colour = fcolour;

	// diffuse lighting calulation
	vec3 diffuse = max(dot(N, L), 0.0) * (diffuse_colour.xyz / diffuse_colour.w);

	vec3 specular =  pow(max(dot(R, V), 0.0), shininess) * specular_colour;


	// fog
	float dist = length(fposition.xyz);
	if (fogmode == 1) 
		fog_factor = (fog_maxdist - dist) / (fog_maxdist - fog_mindist);
	else if (fogmode == 2)
		fog_factor = 1.0 / exp(dist * fogDensity);
	else if (fogmode == 3)
		fog_factor = 1.0 / pow(exp(dist * fogDensity), 2);

	fog_factor = clamp(fog_factor, 0.0, 1.0);
	vec4 c = vec4(fcolour.r * 2.f, fcolour.g * 2.f, fcolour.b * 2.f, fcolour.a);
	// end fog

	outputColor = mix(fog_colour, vec4((fambient + diffuse + specular), 1.0f) , fog_factor) ;


}

