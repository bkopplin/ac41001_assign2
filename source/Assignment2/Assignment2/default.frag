/*
AC41001 Graphics Assignment 2 
Bjarne Kopplin 2021
*/


#version 420

in vec4 fcolour;
in vec2 ftexcoord;
out vec4 outputColor;

layout (binding=0) uniform sampler2D tex1;


void main()
{
	vec4 texcolour = texture(tex1, ftexcoord);
	outputColor = fcolour * texcolour;

}