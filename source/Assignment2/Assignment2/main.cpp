/*
Assignment 2 
Bjarne Kopplin 2021
*/


/* Link to static libraries, could define these as linker inputs in the project settings instead
if you prefer */
#ifdef _DEBUG
#pragma comment(lib, "glfw3D.lib")
#pragma comment(lib, "glloadD.lib")
#else
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glload.lib")
#endif
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "soil.lib")

/* Include the header to the GLFW wrapper class which
   also includes the OpenGL extension initialisation*/
#include "wrapper_glfw.h"
#include "cube_tex.h"
#include "sphere_tex.h"
#include <iostream>

// stb loader
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

#include "tiny_loader.h"
#include <stack>
#include "terrain_object.h"

using namespace std;
using namespace glm;

/* Define buffer object indices */
GLuint positionBufferObject, colourObject, normalsBufferObject;
GLuint sphereBufferObject, sphereNormals, sphereColours, sphereTexCoords;
GLuint elementbuffer;

GLuint program;	
const GLuint NUM_SHADERS = 2; // 1: default, 2: terrain
GLuint shaders[NUM_SHADERS];
GLuint currentShader = 0;

GLuint vao;	
GLuint colourmode;

/* Position and view globals */
GLuint drawmode;	

/* Uniforms*/
GLuint modelID[NUM_SHADERS], viewID[NUM_SHADERS], projectionID[NUM_SHADERS];
GLuint normalmatrixID[NUM_SHADERS];
GLuint cubeTexID;
GLuint sphereTexID;

GLfloat aspect_ratio;	
GLuint numspherevertices;
Sphere moon(true);

TinyObjLoader aircraft;

terrain_object* heightfield;
int octaves;
GLfloat perlin_scale, perlin_frequency;
GLfloat land_size;
GLuint land_resolution;


GLfloat rotateX, rotateY;

vec4 cameraLookAt, cameraPos;
GLfloat aircraft_height;
vec3 aircraft_velocity, aircraft_position;

/* 
* Loads a texture that is stored in filename. 
* filename: the filename of the texture
* texID: the OpenGL texture ID to associate the loaded texture with
* bGenMipmaps: wether or not to generate Mipmaps
*/
bool load_texture(const char* filename, GLuint& texID, bool bGenMipmaps) {

	glGenTextures(1, &texID);

	// Load Textures
	int width, height, nrChannels;
	unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);

	if (data) {
		int pixel_format = 0;
		pixel_format = 0;
		if (nrChannels == 3)
			pixel_format = GL_RGB;
		else
			pixel_format = GL_RGBA;
		glBindTexture(GL_TEXTURE_2D, texID);

		glTexImage2D(GL_TEXTURE_2D, 0, pixel_format, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, data);

		// Generate Mip Maps
		if (bGenMipmaps)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			// If mipmaps are not used then ensure that the min filter is defined
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
	}
	else {
		printf("stb_image loading error: ");
		exit(0);
		return false;
	}
	stbi_image_free(data);
	return true;

}

/*
This function is called before entering the main rendering loop.
Use it for all your initialisation stuff
*/
void init(GLWrapper *glw)
{
	/* Set the object transformation controls to their initial values */
	aspect_ratio = 1.3333f;
	colourmode = 0;
	rotateX = 56;
	rotateY = 144;

	cameraLookAt = vec4(0, 0, 0, 1);
	cameraPos = vec4(0, 10, 4, 1);

	aircraft_position = vec3(5, 5.f, -5);
	aircraft_velocity = vec3(0, 0, 0.001);

	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	moon.makeSphere(60, 60);

	/* Load and build the vertex and fragment shaders */
	try
	{
		shaders[0] = glw->LoadShader("default.vert", "default.frag");
		shaders[1] = glw->LoadShader("terrain.vert", "terrain.frag");
	}
	catch (exception &e)
	{
		cout << "Caught exception: " << e.what() << endl;
		cin.ignore();
		exit(0);
	}

	/* Define uniforms to send to vertex shader */
	for (int i = 0; i < NUM_SHADERS; i++) {
		modelID[i] = glGetUniformLocation(shaders[i], "model");
		viewID[i] = glGetUniformLocation(shaders[i], "view");
		projectionID[i] = glGetUniformLocation(shaders[i], "projection");
		normalmatrixID[i] = glGetUniformLocation(shaders[1], "normalmatrix");

	}

	stbi_set_flip_vertically_on_load(true);

	const char* textureFileMoon = "..//..//images//moon.jpg";
	if (!load_texture(textureFileMoon, sphereTexID, true)) {
		cout << "Fatal error loading texture" << endl;
		exit(0);
	}

	int loc = glGetUniformLocation(shaders[0], "tex1");
	if (loc >= 0) glUniform1i(loc, 0);

	aircraft.load_obj("..//..//objects//star_destroyer.obj");
	aircraft.overrideColour(vec4(0.7f, 0.7f, 0.7f, 1.f));

	/* Create the heightfield object */
	octaves = 4;
	perlin_scale = 12.f;
	perlin_frequency = 10.f;	
	land_size = 40;
	land_resolution = 750;
	heightfield = new terrain_object(octaves, perlin_frequency, perlin_scale);
	heightfield->createTerrain(land_resolution, land_resolution, land_size, land_size);
	heightfield->setColourBasedOnHeight();
	heightfield->createObject();
}

/* Called to update the display. Note that this function is called in the event loop in the wrapper
   class because we registered display as a callback function */
void display()
{
	/* Define the background colour */
	glClearColor(0.1, 0.1, 0.1, 1.0f);

	/* Clear the colour and frame buffers */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Enable depth test  */
	glEnable(GL_DEPTH_TEST);

	/* Make the compiled shader program current */
	currentShader = 0;
	glUseProgram(shaders[currentShader]);

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	mat4 projection = perspective(radians(30.0f), aspect_ratio, 0.1f, 100.0f);

	mat4 view = lookAt(
		vec3(0, 10, 4),
		vec3(0, 0, 0),
		vec3(0, 1, 0)
	);
	view = rotate(view, -radians(rotateX), vec3(1, 0, 0));
	view = rotate(view, -radians(rotateY), vec3(0, 1, 0));
	view = scale(view, vec3(2.f));
	view = translate(view, vec3(-12, -6, 14)); // moves the view to the correct location

	glUniformMatrix4fv(viewID[currentShader], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID[currentShader], 1, GL_FALSE, &projection[0][0]);

	stack<mat4> model;
	model.push(mat4(1.0f));

	/* Moon */
	model.push(model.top());
	{
	
		model.top() = translate(model.top(), vec3(2.0f, 6.f, 4.0f));
		model.top() = scale(model.top(), vec3(0.5f, 0.5f, 0.5f));//scale equally in all axis
		// rotate so that the correct moon texture is facing the viewer
		model.top() = rotate(model.top(), glm::radians(61.f), vec3(0, 1, 0));
		model.top() = rotate(model.top(), glm::radians(-46.f), vec3(1, 0, 0));
		model.top() = rotate(model.top(), glm::radians(5.f), vec3(0, 0, 1));

		glUniformMatrix4fv(modelID[currentShader], 1, GL_FALSE, &model.top()[0][0]);

		glBindTexture(GL_TEXTURE_2D, sphereTexID);
		moon.drawSphere(drawmode);
	}
	model.pop();


	// terrain
	model.push(model.top());
	{
		// use the terrain shader
		currentShader = 1;
		glUseProgram(shaders[currentShader]);
		glUniformMatrix4fv(projectionID[currentShader], 1, GL_FALSE, &projection[0][0]);
		glUniformMatrix4fv(viewID[currentShader], 1, GL_FALSE, &view[0][0]);

		model.top() = translate(model.top(), vec3(-0.05 - 0.5, 0, 0));
		model.top() = scale(model.top(), vec3(1, 1 / 2.f, 1));//scale equally in all axis

		mat4 normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[currentShader], 1, GL_FALSE, &normalmatrix[0][0]);
		glUniformMatrix4fv(modelID[currentShader], 1, GL_FALSE, &model.top()[0][0]);

		heightfield->drawObject(drawmode);
	}
	model.pop();

	// 3D Model Aircraft
	model.push(model.top());
	{
		// use the same shader as the heightfield
		model.top() = translate(model.top(), aircraft_position);
		model.top() = scale(model.top(), vec3(0.15, 0.15, 0.15));

		mat4 normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[currentShader], 1, GL_FALSE, &normalmatrix[0][0]);
		glUniformMatrix4fv(modelID[currentShader], 1, GL_FALSE, &model.top()[0][0]);
		glBindTexture(GL_TEXTURE_2D, 0);

		aircraft.drawObject(drawmode);
	}
	model.pop();



	glDisableVertexAttribArray(0);
	glUseProgram(0);

	/* Modify animation variables */
	aircraft_position += aircraft_velocity; // move the aircraft
}

/* Called whenever the window is resized. The new window size is given, in pixels. */
static void reshape(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	aspect_ratio = ((float)w / 640.f*4.f) / ((float)h / 480.f*3.f);
}

/* change view angle, exit upon ESC */
static void keyCallback(GLFWwindow* window, int key, int s, int action, int mods)
{
	/* Enable this call if you want to disable key responses to a held down key*/
	//if (action != GLFW_PRESS) return;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);


	// Look down
	if (key == 'S') 
		rotateX = glm::clamp(rotateX - 2.f, 50.f, 70.f);

	// Look up
	if (key == 'W')
		rotateX = glm::clamp(rotateX + 2.f, 50.f, 70.f);

	// Move view to the right
	if (key == 'D') 
		rotateY = glm::clamp(rotateY - 2.f, 110.f, 160.f);

	// Move view to the left
	if (key == 'A')
		rotateY = glm::clamp(rotateY + 2.f, 110.f, 160.f);

	/* Cycle between drawing vertices, mesh and filled polygons */
	if (key == ',' && action != GLFW_PRESS)
	{
		drawmode ++;
		if (drawmode > 2) drawmode = 0;
	}

}

void printInstructions() {
	cout << endl
		<< "----------------------------------" << endl
		<< "          INSTRUCTIONS" << endl
		<< "----------------------------------" << endl << endl
		<< "* Look around\t\t -- W, A, S, D" << endl << endl
		<< "* Toggle Drawmode\t -- ," << endl << endl;

}

/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper *glw = new GLWrapper(1024, 768, "Lab5: Fun with texture");;

	if (!ogl_LoadFunctions())
	{
		fprintf(stderr, "ogl_LoadFunctions() failed. Exiting\n");
		return 0;
	}

	glw->setRenderer(display);
	glw->setKeyCallback(keyCallback);
	glw->setReshapeCallback(reshape);

	init(glw);
	printInstructions();
	glw->eventLoop();

	delete(glw);
	return 0;
}
