/*
 Lab5start
 This is an starting project for lab5. The goal for you is to apply texture
 to both the cube and the sphere
 Iain Martin October 2018
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
GLfloat angle_x, angle_inc_x, x, model_scale, z, y;
GLfloat angle_y, angle_inc_y, angle_z, angle_inc_z;
GLuint drawmode;	
GLuint numlats, numlongs;

/* Uniforms*/
GLuint modelID[NUM_SHADERS], viewID[NUM_SHADERS], projectionID[NUM_SHADERS];
GLuint colourmodeID[NUM_SHADERS];
GLuint cubeTexID;
GLuint sphereTexID;

GLfloat aspect_ratio;	
GLuint numspherevertices;
Cube aCube(true);
Sphere moon(true);

TinyObjLoader aircraft;

terrain_object* heightfield;
int octaves;
GLfloat perlin_scale, perlin_frequency;
GLfloat land_size;
GLuint land_resolution;


GLfloat rotateX, rotateY, moveFW, moveSW, moveUP, viewScale;

GLfloat debug_goup;

vec4 cameraLookAt, cameraPos;
GLfloat aircraft_height;
vec3 aircraft_velocity, aircraft_position;



bool load_texture(char* filename, GLuint& texID, bool bGenMipmaps);

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
	debug_goup = 0;
	/* Set the object transformation controls to their initial values */
	x = 0.05f;
	y = 0;
	z = 0;
	angle_x = angle_y = angle_z = 0;
	viewScale = 2.f;
	angle_inc_x = angle_inc_y = angle_inc_z = 0;
	model_scale = 1.f;
	aspect_ratio = 1.3333f;
	colourmode = 0;
	numlats = 60;		// Number of latitudes in our sphere
	numlongs = 60;		// Number of longitudes in our sphere
	rotateX = 40;
	rotateY = 170;
	moveFW = 8.f;
	moveSW = 0.0;
	moveUP = -6.0;

	cameraLookAt = vec4(0, 0, 0, 1);
	cameraPos = vec4(0, 10, 4, 1);

	aircraft_height = 2.f;
	aircraft_position = vec3(0, 0, 0);
	aircraft_velocity = vec3(0, 0, 0.001);

	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	/* create the sphere and cube objects */
	moon.makeSphere(numlats, numlongs);

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
		colourmodeID[i] = glGetUniformLocation(shaders[i], "colourmode");
	}


	// Enable face culling. This will cull the back faces of all
	// triangles. Be careful to ensure that triangles are drawn
	// with correct winding.
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);

	stbi_set_flip_vertically_on_load(true);

	const char* textureFileMoon = "..//..//images//moon.jpg";
	if (!load_texture(textureFileMoon, sphereTexID, true)) {
		cout << "Fatal error loading texture" << endl;
		exit(0);
	}

	int loc = glGetUniformLocation(shaders[0], "tex1");
	if (loc >= 0) glUniform1i(loc, 0);

	aircraft.load_obj("..//..//objects//airbus.obj");
	aircraft.overrideColour(vec4(0.7f, 0.7f, 0.7f, 1.f));

	/* Create the heightfield object */
	octaves = 8;
	perlin_scale = 8.5f;
	perlin_frequency = 8.2f;
	land_size = 10;
	land_resolution = 1450;
	heightfield = new terrain_object(octaves, perlin_frequency, perlin_scale);
	heightfield->createTerrain(land_resolution, land_resolution, land_size, land_size);
	//heightfield->setColour(vec3(1, 0.2, 0.0));
	heightfield->setColourBasedOnHeight();
	heightfield->createObject();
}

/* Called to update the display. Note that this function is called in the event loop in the wrapper
   class because we registered display as a callback function */
void display()
{
	/* Define the background colour */
	glClearColor(0.4f, 0.4f, 0.4f, 1.0f);

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
	view = scale(view, vec3(viewScale));
	view = translate(view, vec3(moveSW, moveUP, moveFW));

	glUniform1ui(colourmodeID[currentShader], colourmode);
	glUniformMatrix4fv(viewID[currentShader], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID[currentShader], 1, GL_FALSE, &projection[0][0]);

	// Define the model transformations for the cube
	stack<mat4> model;
	model.push(mat4(1.0f));

	/* Moon */
	model.push(model.top());
	{
	
		model.top() = translate(model.top(), vec3(2.0f, 4.f, 4.0f));
		model.top() = scale(model.top(), vec3(0.5f, 0.5f, 0.5f));//scale equally in all axis

		glUniformMatrix4fv(modelID[currentShader], 1, GL_FALSE, &model.top()[0][0]);

		glBindTexture(GL_TEXTURE_2D, sphereTexID);
		moon.drawSphere(drawmode);
	}
	model.pop();


	// terrain
	model.push(model.top());
	{
		currentShader = 1;
		glUseProgram(shaders[currentShader]);
		// Send our uniforms variables to the currently bound shader,
		glUniform1ui(colourmodeID[currentShader], colourmode);
		glUniformMatrix4fv(projectionID[currentShader], 1, GL_FALSE, &projection[0][0]);
		glUniformMatrix4fv(viewID[currentShader], 1, GL_FALSE, &view[0][0]);

		model.top() = translate(model.top(), vec3(-x - 0.5, y, 0));
		model.top() = scale(model.top(), vec3(model_scale, model_scale / 2.f, model_scale));//scale equally in all axis

		glUniformMatrix4fv(modelID[currentShader], 1, GL_FALSE, &model.top()[0][0]);

		heightfield->drawObject(drawmode);
	}
	model.pop();

	// 3D Model Aircraft
	model.push(model.top());
	{
		model.top() = translate(model.top(), aircraft_position);
		model.top() = translate(model.top(), vec3(0, aircraft_height, 0));
		model.top() = scale(model.top(), vec3(0.15, 0.15, 0.15));

		glUniformMatrix4fv(modelID[currentShader], 1, GL_FALSE, &model.top()[0][0]);
		glBindTexture(GL_TEXTURE_2D, 0);

		aircraft.drawObject(drawmode);
	}
	model.pop();



	glDisableVertexAttribArray(0);
	glUseProgram(0);

	/* Modify animation variables */
	aircraft_position += aircraft_velocity;
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



	if (key == 'W') {
		rotateX = glm::clamp(rotateX - 2.f, -90.f, 90.f);
		//mat4 lookatTransform = mat4(1.f);
		//mat4 transform = mat4(1.f);
		//transform = rotate(mat4(1.0f), radians(2.f), vec3(0, 1, 0));
		//transform = translate(mat4(1.f), vec3(1, 0, 0));

		////cameraLookAt = rotate(mat4(1.0f), radians(5.f), vec3(1, 0, 0)) * cameraLookAt;
		//cameraLookAt = transform * cameraLookAt;
	}

	if (key == 'S')
		rotateX = glm::clamp(rotateX + 2.f, -90.f, 90.f);


	if (key == 'D') {
		cameraPos = translate(mat4(1.f), vec3(1, 0, 0)) * cameraPos;
		rotateY += 2.f;
	}

	if (key == 'A')
		rotateY -= 2.f;

	if (key == GLFW_KEY_UP)
		moveFW += 1.f;

	if (key == GLFW_KEY_DOWN)
		moveFW -= 1.f;


	if (key == GLFW_KEY_LEFT)
		moveSW += 1.f;

	if (key == GLFW_KEY_RIGHT)
		moveSW -= 1.f;

	if (key == '9')
		moveUP += 1.f;

	if (key == '0')
		moveUP -= 1.f;


	if (key == GLFW_KEY_F1)
		debug_goup += 0.2f;

	if (key == 'Q') angle_x -= 1.f;
	if (key == 'P') angle_x += 1.f;
	if (key == 'E') angle_inc_y -= 0.05f;
	if (key == 'R') angle_inc_y += 0.05f;
	if (key == 'T') angle_inc_z -= 0.05f;
	if (key == 'Y') angle_inc_z += 0.05f;
	if (key == 'Z') x -= 0.5f;
	if (key == 'X') x += 0.5f;
	if (key == 'C') y -= 0.2f;
	if (key == 'V') y += 0.2f;
	if (key == 'B') z -= 0.2f;
	if (key == 'N') z += 0.2f;

	if (key == '-')
		viewScale -= 0.2f;

	if (key == ']')
		viewScale += 0.2f;	

	if (key == 'M' && action != GLFW_PRESS)
	{
		colourmode = !colourmode;
	}

	/* Cycle between drawing vertices, mesh and filled polygons */
	if (key == ',' && action != GLFW_PRESS)
	{
		drawmode ++;
		if (drawmode > 2) drawmode = 0;
	}
	cout << "moveFW: " << moveFW << ", moveUP: " << moveUP << ", moveSW: " << moveSW << endl;
	cout << "rotateX: " << rotateX << ", rotateY: " << rotateY << endl;
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

	glw->eventLoop();

	delete(glw);
	return 0;
}
