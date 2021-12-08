// Vertex shader with Gouraud shading
// Iain Martin 2018

#version 420

// These are the vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 colour;
layout(location = 2) in vec3 normal;

// Uniform variables are passed in from the application
uniform mat4 model, view, projection;
uniform uint colourmode;
uniform mat3 normalmatrix;

out vec3 fambient;

out vec3 N, L, R, V;

// Output the vertex colour - to be rasterized into pixel fragments
out vec4 fcolour;
out vec4 fposition;
vec4 ambient = vec4(0.2, 0.2,0.2,1.0);
vec3 light_dir = vec3(0.0, 10.0, 10.0);

void main()
{
//	vec4 specular_colour = vec4(0.0,0.0,0.0,1.0);
//	vec4 diffuse_colour = vec4(0.5,0.5,0,1.0);
	vec4 position_h = vec4(position, 1.0);
//	float shininess = 8.0;
//	
//	// Switch between using the vertex colour buffer colours
//	// or brown. 
//	if (colourmode == 1)
//	{
//		diffuse_colour = vec4(0.6, 0.4, 0.2, 1.0);
//
//	}
//	else
//	{
//		diffuse_colour = vec4(colour, 1.0);
//	}
//
//	ambient = diffuse_colour * 0.2;
//
//	mat4 mv_matrix = view * model;
//	mat3 normalmatrix = mat3(mv_matrix);
//	vec3 N = mat3(mv_matrix) * normal;
//	N = normalize(N);
//	light_dir = normalize(light_dir);
//
//	vec4 diffuse = max(dot(N, light_dir), 0.0) * diffuse_colour;
//
//	vec4 P = position_h * mv_matrix;
//	vec3 half_vec = normalize(light_dir + P.xyz);
//	vec4 specular = pow(max(dot(N, half_vec), 0.0), shininess) * specular_colour;


	// fragment lighting
		vec4 pos_h = vec4(position, 1.0);
	vec4 diffuse_colour = vec4(colour, 1); 
	
	mat4 mv = view * model; 
	vec4 P = mv * pos_h;
	vec3 P_c = P.xyz / P.w;

	// diffuse light
	N = normalize(normalmatrix * normal);
	L = normalize( light_dir );

	// specular light
	V = normalize(-P.xyz);
	R = reflect(-L, N);

	// ambient
	

	fambient = colour.xyz * 0.4f;
	fcolour = vec4(colour, 1);

	// Define the vertex colour
	//fcolour = diffuse + ambient + specular;

	// Define the vertex position
	fposition = gl_Position = projection * view * model * position_h;
}

