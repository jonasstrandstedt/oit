/**
Copyright (C) 2012-2014 Jonas Strandstedt

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "Engine.h"
#include "Sphere.h"
#include "Model.h"

// objects
gl4::VBO *obj;
gl4::Sphere *sphere;
gl4::Engine *engine;
gl4::Model *bunny;

// global settings
glm::vec2 angle;
bool wireframe = false;

// callback functions
void keyboardCallback(int key, int state);
void myRenderFunc(void);
void myInitFunc(void);
void myUpdateFunc(float dt);

// OIT
#define MAX_FRAMEBUFFER_WIDTH 2048
#define MAX_FRAMEBUFFER_HEIGHT 2048
GLuint *data;
size_t total_pixels = MAX_FRAMEBUFFER_WIDTH * MAX_FRAMEBUFFER_HEIGHT;
GLuint head_pointer_texture;
GLuint head_pointer_initializer;
GLuint atomic_counter_buffer;
GLuint fragment_storage_buffer;
GLuint linked_list_texture;
GLuint sort_shader;
GLuint dispatch_buffer;

const GLchar * sort_source [] = {
R"(#version 430

layout(local_size_x = 16, local_size_y = 16) in;

layout (binding = 0, r32ui) uniform uimage2D head_pointer_image;
layout (binding = 1, rgba32ui) uniform uimageBuffer list_buffer;

void main() {
	
	uint current = imageLoad(head_pointer_image, ivec2(gl_GlobalInvocationID.xy)).x;

	while(current != 0) {
		uvec4 item_i = imageLoad(list_buffer, int(current));

		uint next = item_i.x;
		while(next != 0) {
			uvec4 item_j = imageLoad(list_buffer, int(next));

			float depth_i = uintBitsToFloat(item_i.z);
			float depth_j = uintBitsToFloat(item_j.z);

			if(depth_i > depth_j) {
				uvec3 tmp = item_i.yzw;
				item_i.yzw = item_j.yzw;
				item_j.yzw = tmp;
				imageStore(list_buffer, int(current), item_i);
				imageStore(list_buffer, int(next), item_j);
			}

			next = item_j.x;
		}
		current = item_i.x;
	}
};

)"};

int main(int argc, char **argv) {

	// create engine object
	engine = new gl4::Engine(argc, argv);
	
	// set callbacks
	engine->setKeyBoardCallbackfunc(keyboardCallback);
	engine->setUpdateFunc(myUpdateFunc);
	engine->setRenderFunc(myRenderFunc);
	engine->setInitFunc(myInitFunc);
	
	// init
	if( ! engine->initGL()) {
		return 1;
	}

	// do render
	engine->render();

	// cleanup
	delete sphere;
	delete obj;
	delete engine;

	// return success
	return 0;
}

void keyboardCallback(int key, int state) 
{
	// toggle wireframe
	if(key == 'W' && state == GLFW_PRESS) {
		wireframe = ! wireframe;
	}
}


void myInitFunc(void) 
{
	// init gl states
	glDisable (GL_DEPTH_TEST);

	angle[0] = 0.0f;
	angle[1] = 0.0f;

	// init plane
	obj = new gl4::VBO();
	obj->setProportions(800, 600);
	obj->init();

	// init sphere
	sphere = new gl4::Sphere(1.0, 30);
	sphere->init();

	// init bunny
	bunny = new gl4::Model("../data/obj/bunny.obj", "../data/obj/bunny.png",glm::vec3(0.0,0.0,0.0),0.05,glm::vec3(0.0,0.0,0.0));
	bunny->init();

	// load textures
	gl4::TextureManager::getInstance()->loadTexture("earth_diffuse", "../data/earth_nasa_lowres.tga");

	gl4::Shader *passthrough = new gl4::Shader("../shaders/passthrough_VS.glsl", "../shaders/passthrough_FS.glsl");
	gl4::Shader *oit_render = new gl4::Shader("../shaders/oit_render_VS.glsl", "../shaders/oit_render_FS.glsl");
	gl4::Shader *oit_sortdraw = new gl4::Shader("../shaders/oit_sortdraw_VS.glsl", "../shaders/oit_sortdraw_FS.glsl");
	gl4::ShaderManager::getInstance()->addShaderProgram("passthrough", passthrough);
	gl4::ShaderManager::getInstance()->addShaderProgram("oit_render", oit_render);
	gl4::ShaderManager::getInstance()->addShaderProgram("oit_sortdraw", oit_sortdraw);


	// OIT
	glGenTextures(1, &head_pointer_texture);
	glBindTexture(GL_TEXTURE_2D, head_pointer_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, MAX_FRAMEBUFFER_WIDTH, MAX_FRAMEBUFFER_HEIGHT, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);

	glGenBuffers(1, &head_pointer_initializer);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, head_pointer_initializer);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, total_pixels * sizeof(GLuint), NULL, GL_STATIC_DRAW);

	data = (GLuint*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
	memset(data, 0x00, total_pixels * sizeof(GLuint));
	glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

	glGenBuffers(1, &atomic_counter_buffer);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomic_counter_buffer);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_COPY);

	glGenBuffers(1, &fragment_storage_buffer);
	glBindBuffer(GL_TEXTURE_BUFFER, fragment_storage_buffer);
	glBufferData(GL_TEXTURE_BUFFER, 4*total_pixels*sizeof(GLfloat)*4, NULL, GL_DYNAMIC_COPY);

    glGenTextures(1, &linked_list_texture);
    glBindTexture(GL_TEXTURE_BUFFER, linked_list_texture);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32UI, fragment_storage_buffer);
    glBindTexture(GL_TEXTURE_BUFFER, 0);

    glBindImageTexture(1, linked_list_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32UI);


   	GLuint sort_shader_part = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(sort_shader_part, 1, sort_source, NULL);
	glCompileShader(sort_shader_part);

	sort_shader = glCreateProgram();
	glAttachShader(sort_shader, sort_shader_part);
	glLinkProgram(sort_shader);

	char str[4096];
	GLint shadersLinked = GL_FALSE;

	// Link the program object and print out the info log
	glGetProgramiv( sort_shader, GL_LINK_STATUS, &shadersLinked );

	if( shadersLinked == GL_FALSE )
	{
		glGetInfoLogARB( sort_shader, sizeof(str), NULL, str );
		ERRLOG("Program object linking error: %s\n", str);
	} 

	glUseProgram(sort_shader);

	glGenBuffers(1, & dispatch_buffer);
	glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, dispatch_buffer);

	static const struct
	{
		GLuint num_groups_x;
		GLuint num_groups_y;
		GLuint num_groups_z;
	} dispatch_params = { MAX_FRAMEBUFFER_WIDTH / 16, MAX_FRAMEBUFFER_HEIGHT / 16, 1};

	glBufferData(GL_DISPATCH_INDIRECT_BUFFER, sizeof(dispatch_params), &dispatch_params, GL_STATIC_DRAW);
	glUseProgram(0);


    //printf("source: %s\n", sort_source[0]);



}

void myRenderFunc(void) 
{
	float s = 5.0;
	
	/*
	gl4::ShaderManager::getInstance()->bindShader("passthrough");
	glm::mat4 bunny_transform = glm::mat4(1.0);
	bunny_transform = glm::scale(bunny_transform,glm::vec3(s,s,s));
	bunny_transform = glm::rotate(bunny_transform,angle[1], glm::vec3(0.0f, 1.0f, 0.0f));
	bunny_transform = glm::rotate(bunny_transform,angle[0], glm::vec3(1.0f, 0.0f, 0.0f));
	engine->usePerspectiveProjection(bunny_transform);
	bunny->render();

	gl4::ShaderManager::getInstance()->unbindShader();
*/

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, head_pointer_initializer);
	glBindTexture(GL_TEXTURE_2D, head_pointer_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, MAX_FRAMEBUFFER_WIDTH, MAX_FRAMEBUFFER_HEIGHT, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);

	glBindImageTexture(0, head_pointer_texture, 0,GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomic_counter_buffer);

	const GLuint zero = 0;
	glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(zero), &zero);



	gl4::ShaderManager::getInstance()->bindShader("oit_render");
	glm::mat4 bunny_transform = glm::mat4(1.0);
	bunny_transform = glm::scale(bunny_transform,glm::vec3(s,s,s));
	bunny_transform = glm::rotate(bunny_transform,angle[1], glm::vec3(0.0f, 1.0f, 0.0f));
	bunny_transform = glm::rotate(bunny_transform,angle[0], glm::vec3(1.0f, 0.0f, 0.0f));
	engine->usePerspectiveProjection(bunny_transform);

	// Bind head-pointer image for read-write
    glBindImageTexture(0, head_pointer_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

    // Bind linked-list buffer for write
    glBindImageTexture(1, linked_list_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32UI);

	bunny->render();
	gl4::ShaderManager::getInstance()->unbindShader();

	glUseProgram(sort_shader);
	//glDispatchCompute(16,16,1);
	glDispatchComputeIndirect(0);
	glUseProgram(0);



	gl4::ShaderManager::getInstance()->bindShader("oit_sortdraw");

	engine->useOrthogonalProjection();

	obj->render();
	gl4::ShaderManager::getInstance()->unbindShader();

/*
	unsigned int val = 0;
	glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(val), &val);
	printf("atomic_counter_buffer: %i\n", val);
*/
}


void myUpdateFunc(float dt)
{
	float speed = 50.0f;
	if(engine->isKeyPressed(GLFW_KEY_RIGHT)) {
		angle[1] += dt*speed;
	}
	if(engine->isKeyPressed(GLFW_KEY_LEFT)) {
		angle[1] -= dt*speed;
	}
	if(engine->isKeyPressed(GLFW_KEY_UP)) {
		angle[0] += dt*speed;
	}
	if(engine->isKeyPressed(GLFW_KEY_DOWN)) {
		angle[0] -= dt*speed;
	}
}