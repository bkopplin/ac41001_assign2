/*
AC41001 Graphics Assignment 2 
Bjarne Kopplin 2021
*/

#version 420

// These are the vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 texcoord;

// Uniform variables are passed in from the application
uniform mat4 model, view, projection;
uniform uint colourmode;

// Output the vertex colour - to be rasterized into pixel fragments
out vec2 ftexcoord;
out vec4 fcolour;
out vec4 fposition;
vec4 ambient = vec4(0.2, 0.2,0.2,1.0);
vec3 light_dir = vec3(0.0, 0.0, 10.0);

void main()
{
	vec4 specular_colour = vec4(1.0,1.0,1.0,1.0);
	vec4 diffuse_colour = vec4(0.5,0.5,0,1.0);
	vec4 position_h = vec4(position, 1.0);
	float shininess = 8.0;


	diffuse_colour = vec4(1.0, 1.0, 1, 1.0);

	// ambient lighting
	ambient = diffuse_colour * 0.2;

	// calculte diffuse lighting
	mat4 mv_matrix = view * model;
	mat3 n_matrix = transpose(inverse(mat3(mv_matrix)));
	vec3 N = normalize(n_matrix * normal);
	vec3 L = normalize(light_dir);

	vec4 diffuse = max(dot(N, L), 0.0) * diffuse_colour;

	// pass values to fragment shader
	fcolour = diffuse + ambient;
	ftexcoord = texcoord;
	fposition = gl_Position = projection * view * model * position_h;
}

