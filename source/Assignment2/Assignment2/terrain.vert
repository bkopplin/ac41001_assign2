/*
AC41001 Graphics Assignment 2 
Bjarne Kopplin 2021
*/


#version 420

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 colour;
layout(location = 2) in vec3 normal;

// Uniform variables 
uniform mat4 model, view, projection;
uniform mat3 normalmatrix;

out vec3 fambient;
out vec3 N, L, R, V;
out vec4 fcolour;
out vec4 fposition;

vec4 ambient = vec4(0.2, 0.2,0.2,1.0);
vec3 light_dir = vec3(2.0f, 6.f, 4.0f); // same as the moons position.

void main()
{

	vec4 position_h = vec4(position, 1.0);


	// fragment lighting
	vec4 pos_h = vec4(position, 1.0);
	mat4 mv = view * model; 
	vec4 P = mv * pos_h;

	// diffuse light
	N = normalize(normalmatrix * normal);
	L = normalize( light_dir );

	// specular light
	V = normalize(-P.xyz);
	R = reflect(-L, N);

	// ambient
	

	fambient = colour.xyz * 0.4f;
	fcolour = vec4(colour, 1);

	// Define the vertex position
	fposition = gl_Position = projection * view * model * position_h;
}

