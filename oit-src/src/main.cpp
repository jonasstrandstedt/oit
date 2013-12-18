/**
Copyright (C) 2012-2014 Jonas Strandstedt

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "Engine.h"
#include "Sphere.h"
#include "Model.h"
#include "main.h"


// objects
gl4::VBO *obj;
gl4::VBO *plane;
gl4::Sphere *sphere;
gl4::Engine *engine;
gl4::Model *bunny;
gl4::Model *armadillo;
gl4::Model *dragon;
gl4::Model *box;
glm::mat4 camera_transform;
glm::vec3 camera_position;
glm::vec3 camera_rotation;

// global settings
glm::vec2 angle;
glm::vec3 bunny_position;
bool wireframe = false;

// callback functions
void keyboardCallback(int key, int state);
void myRenderFunc(void);
void myInitFunc(void);
void myUpdateFunc(float dt);

// OIT
#define MAX_FRAMEBUFFER_WIDTH 800
#define MAX_FRAMEBUFFER_HEIGHT 600
#define MAX_FRAMEBUFFER_SAMPLES 10
GLuint *data;
GLfloat *dataf;
size_t total_pixels = MAX_FRAMEBUFFER_WIDTH * MAX_FRAMEBUFFER_HEIGHT;
GLuint head_pointer_texture;
GLuint head_pointer_initializer;
GLuint atomic_counter_buffer;
GLuint atomic_counter_array_buffer_texture;
GLuint atomic_counter_array_buffer;
GLuint accumulated_counter_array_buffer_texture;
GLuint accumulated_counter_array_buffer;
GLuint fragment_storage_buffer;
GLuint linked_list_texture;
GLuint sort_shader;
GLuint render_shader;
GLuint dispatch_buffer;
GLuint loc_base_color;
unsigned int * get_pixel_buffer;
float * get_dof_buffer;

// DOF
#define DOF_LAYERS 1
GLuint dof_initializer;
GLuint dof_textures;
GLuint dof_temp;
GLuint dof_final;
gl4::FBO ping, pong, fbo_tmp, fbo_final;
GLuint dof_render_shader;
GLuint dof_combine_shader;



void render_to_final(GLuint texture);
void render_scene();

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
	delete bunny;
	delete armadillo;
	delete dragon;
	delete box;
	delete sphere;
	delete obj;
	delete engine;
	delete[] get_pixel_buffer;

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
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D_ARRAY);
	glfwSwapInterval(0);

	// camera position
	camera_position[1] = 10.0;
	camera_position[2] = 0.5;


	// init plane
	obj = new gl4::VBO();
	obj->setProportions(800, 600);
	obj->init();
	plane = new gl4::VBO();
	//wall->setProportions(2, 2);
	plane->init();

	// init sphere
	sphere = new gl4::Sphere(1.0, 30);
	sphere->init();

	// init bunny
	bunny = new gl4::Model("../data/obj/bunny.obj", "../data/obj/bunny.png",glm::vec3(0.0,0.0,0.0),0.05,glm::vec3(0.0,0.0,0.0));
	bunny->init();
	
	armadillo = new gl4::Model("../data/obj/armadillo.obj", "../data/obj/armadillo.png",glm::vec3(0.0,0.0,0.0),0.05,glm::vec3(0.0,0.0,0.0));
	armadillo->init();
	dragon = new gl4::Model("../data/obj/dragon.obj", "../data/obj/dragon.png",glm::vec3(0.0,0.0,0.0),0.05,glm::vec3(0.0,0.0,0.0));
	dragon->init();
	box = new gl4::Model("../data/obj/box.obj", "../data/obj/box.png",glm::vec3(0.0,0.0,0.0),0.05,glm::vec3(0.0,0.0,0.0));
	box->init();

	// load textures
	gl4::TextureManager::getInstance()->loadTexture("checkerboard", "../data/checkerboard.png");

	gl4::Shader *passthrough = new gl4::Shader("../shaders/passthrough_VS.glsl", "../shaders/passthrough_FS.glsl");
	gl4::Shader *oit_render = new gl4::Shader("../shaders/oit_render_VS.glsl", "../shaders/oit_render_FS.glsl");
	gl4::Shader *oit_sortdraw = new gl4::Shader("../shaders/oit_sortdraw_VS.glsl", "../shaders/oit_sortdraw_FS.glsl");
	gl4::Shader *oit_counter = new gl4::Shader("../shaders/oit_counter_VS.glsl", "../shaders/oit_counter_FS.glsl");
	gl4::Shader *quad_texture = new gl4::Shader("../shaders/quad_texture_VS.glsl", "../shaders/quad_texture_FS.glsl");
	gl4::Shader *blur_x = new gl4::Shader("../shaders/quad_blur_x_VS.glsl", "../shaders/quad_blur_x_FS.glsl");
	gl4::Shader *blur_y = new gl4::Shader("../shaders/quad_blur_y_VS.glsl", "../shaders/quad_blur_y_FS.glsl");

	gl4::ShaderManager::getInstance()->addShaderProgram("passthrough", passthrough);
	gl4::ShaderManager::getInstance()->addShaderProgram("oit_render", oit_render);
	gl4::ShaderManager::getInstance()->addShaderProgram("oit_sortdraw", oit_sortdraw);
	gl4::ShaderManager::getInstance()->addShaderProgram("oit_counter", oit_counter);
	gl4::ShaderManager::getInstance()->addShaderProgram("quad_texture", quad_texture);
	gl4::ShaderManager::getInstance()->addShaderProgram("blur_x", blur_x);
	gl4::ShaderManager::getInstance()->addShaderProgram("blur_y", blur_y);

	loc_base_color = glGetUniformLocation( oit_render->getShaderProgram(), "base_color");

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

	glGenBuffers(1, &atomic_counter_array_buffer);
	glBindBuffer(GL_TEXTURE_BUFFER, atomic_counter_array_buffer);
	glBufferData(GL_TEXTURE_BUFFER, total_pixels*sizeof(GLuint), NULL, GL_DYNAMIC_COPY);

	glGenTextures(1, &atomic_counter_array_buffer_texture);
	glBindTexture(GL_TEXTURE_2D, atomic_counter_array_buffer_texture);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, fragment_storage_buffer);
    glBindTexture(GL_TEXTURE_BUFFER, 0);

	glGenBuffers(1, &atomic_counter_buffer);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomic_counter_buffer);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_COPY);

	glGenBuffers(1, &fragment_storage_buffer);
	glBindBuffer(GL_TEXTURE_BUFFER, fragment_storage_buffer);
	glBufferData(GL_TEXTURE_BUFFER, MAX_FRAMEBUFFER_SAMPLES*total_pixels*sizeof(GLfloat)*4, NULL, GL_DYNAMIC_COPY);

    glGenTextures(1, &linked_list_texture);
    glBindTexture(GL_TEXTURE_BUFFER, linked_list_texture);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32UI, fragment_storage_buffer);
    glBindTexture(GL_TEXTURE_BUFFER, 0);

    glBindImageTexture(1, linked_list_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32UI);


    // COMPUTE SHADERS
    static const struct
	{
		GLuint num_groups_x;
		GLuint num_groups_y;
		GLuint num_groups_z;
	} dispatch_params = { MAX_FRAMEBUFFER_WIDTH / 16, MAX_FRAMEBUFFER_HEIGHT / 16, 1};

    // SORT SHADER
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

	glBufferData(GL_DISPATCH_INDIRECT_BUFFER, sizeof(dispatch_params), &dispatch_params, GL_STATIC_DRAW);
	glUseProgram(0);

	// RENDER SHADER
	GLuint render_shader_part = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(render_shader_part, 1, render_source, NULL);
	glCompileShader(render_shader_part);

	render_shader = glCreateProgram();
	glAttachShader(render_shader, render_shader_part);
	glLinkProgram(render_shader);

	shadersLinked = GL_FALSE;

	// Link the program object and print out the info log
	glGetProgramiv( render_shader, GL_LINK_STATUS, &shadersLinked );

	if( shadersLinked == GL_FALSE )
	{
		glGetInfoLogARB( render_shader, sizeof(str), NULL, str );
		ERRLOG("Program object linking error: %s\n", str);
	} 

	glUseProgram(render_shader);

	glGenBuffers(1, & dispatch_buffer);
	glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, dispatch_buffer);

	glBufferData(GL_DISPATCH_INDIRECT_BUFFER, sizeof(dispatch_params), &dispatch_params, GL_STATIC_DRAW);
	glUseProgram(0);

	// DOF RENDER SHADER
	GLuint dof_render_shader_part = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(dof_render_shader_part, 1, render_dof_source, NULL);
	glCompileShader(dof_render_shader_part);

	dof_render_shader = glCreateProgram();
	glAttachShader(dof_render_shader, dof_render_shader_part);
	glLinkProgram(dof_render_shader);

	shadersLinked = GL_FALSE;

	// Link the program object and print out the info log
	glGetProgramiv( dof_render_shader, GL_LINK_STATUS, &shadersLinked );

	if( shadersLinked == GL_FALSE )
	{
		glGetInfoLogARB( dof_render_shader, sizeof(str), NULL, str );
		ERRLOG("Program object linking error: %s\n", str);
	} 

	glUseProgram(dof_render_shader);

	glGenBuffers(1, & dispatch_buffer);
	glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, dispatch_buffer);

	glBufferData(GL_DISPATCH_INDIRECT_BUFFER, sizeof(dispatch_params), &dispatch_params, GL_STATIC_DRAW);
	glUseProgram(0);

	// DOF COMBINE SHADER
	GLuint dof_combine_shader_part = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(dof_combine_shader_part, 1, combine_dof_source, NULL);
	glCompileShader(dof_combine_shader_part);

	dof_combine_shader = glCreateProgram();
	glAttachShader(dof_combine_shader, dof_combine_shader_part);
	glLinkProgram(dof_combine_shader);

	shadersLinked = GL_FALSE;

	// Link the program object and print out the info log
	glGetProgramiv( dof_combine_shader, GL_LINK_STATUS, &shadersLinked );

	if( shadersLinked == GL_FALSE )
	{
		glGetInfoLogARB( dof_combine_shader, sizeof(str), NULL, str );
		ERRLOG("Program object linking error: %s\n", str);
	} 

	glUseProgram(dof_combine_shader);

	glGenBuffers(1, & dispatch_buffer);
	glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, dispatch_buffer);

	glBufferData(GL_DISPATCH_INDIRECT_BUFFER, sizeof(dispatch_params), &dispatch_params, GL_STATIC_DRAW);
	glUseProgram(0);

	get_pixel_buffer = new unsigned int[total_pixels];
	get_dof_buffer = new float[total_pixels*4*DOF_LAYERS];


    //printf("source: %s\n", sort_source[0]);
    printf("loc_base_color: %i\n", loc_base_color);

    //DOF
	glGenBuffers(1, &dof_initializer);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, dof_initializer);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, total_pixels * 4*sizeof(GLfloat)*DOF_LAYERS, NULL, GL_STATIC_DRAW);

	dataf = (GLfloat*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
	memset(dataf, 0x00, total_pixels *4* sizeof(GLfloat));
	glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	
	glGenTextures(1, &dof_textures);
	glBindTexture(GL_TEXTURE_2D, dof_textures);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, MAX_FRAMEBUFFER_WIDTH, MAX_FRAMEBUFFER_HEIGHT,0, GL_RGBA, GL_FLOAT, NULL);

	glGenTextures(1, &dof_final);
	glBindTexture(GL_TEXTURE_2D, dof_final);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, MAX_FRAMEBUFFER_WIDTH, MAX_FRAMEBUFFER_HEIGHT,0, GL_RGBA, GL_FLOAT, NULL);

	glGenTextures(1, &dof_temp);
	glBindTexture(GL_TEXTURE_2D, dof_temp);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, MAX_FRAMEBUFFER_WIDTH, MAX_FRAMEBUFFER_HEIGHT,0, GL_RGBA, GL_FLOAT, NULL);

	ping.setTextureFormat(GL_RGBA32F);
	pong.setTextureFormat(GL_RGBA32F);
	fbo_final.setTextureFormat(GL_RGBA32F);
	fbo_tmp.setTextureFormat(GL_RGBA32F);
	ping.init(800,600);
	pong.init(800,600);
	fbo_final.init(800,600);
	fbo_tmp.init(800,600);


	/*
	for (int i = 0; i < DOF_LAYERS; ++i)
	{
		glBindTexture(GL_TEXTURE_2D, dof_textures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, MAX_FRAMEBUFFER_WIDTH, MAX_FRAMEBUFFER_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	}
	*/
	
	
    
    


	engine->glCheckError();
    printf("Init done!\n");

}

void myRenderFunc(void) 
{
	
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, dof_initializer);
	glBindTexture(GL_TEXTURE_2D, dof_textures);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, MAX_FRAMEBUFFER_WIDTH, MAX_FRAMEBUFFER_HEIGHT,0, GL_RGBA, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, dof_final);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, MAX_FRAMEBUFFER_WIDTH, MAX_FRAMEBUFFER_HEIGHT,0, GL_RGBA, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, dof_temp);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, MAX_FRAMEBUFFER_WIDTH, MAX_FRAMEBUFFER_HEIGHT,0, GL_RGBA, GL_FLOAT, NULL);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	fbo_final.clear();
	ping.clear();
	pong.clear();
	fbo_tmp.clear();

	// bind buffer initializer
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, head_pointer_initializer);

	// clear head_pointer
	glBindTexture(GL_TEXTURE_2D, head_pointer_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, MAX_FRAMEBUFFER_WIDTH, MAX_FRAMEBUFFER_HEIGHT, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);

	// clear atomic_counter_array_buffer
	glBindTexture(GL_TEXTURE_2D, atomic_counter_array_buffer_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, MAX_FRAMEBUFFER_WIDTH, MAX_FRAMEBUFFER_HEIGHT, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	// Bind head-pointer image for read-write
    glBindImageTexture(0, head_pointer_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(1, linked_list_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32UI);
    glBindImageTexture(2, atomic_counter_array_buffer_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);



	// count fragments
	gl4::ShaderManager::getInstance()->bindShader("oit_counter");
	glUniformMatrix4fv(4, 1, GL_FALSE, &camera_transform[0][0]);
	render_scene();
	gl4::ShaderManager::getInstance()->unbindShader();

	// fetch and accumulate buffer offset
	glBindTexture(GL_TEXTURE_2D, atomic_counter_array_buffer_texture);
	glGetTexImage(GL_TEXTURE_2D,0,GL_RED_INTEGER,GL_UNSIGNED_INT,get_pixel_buffer);
	for (unsigned int i = 1; i < total_pixels; ++i) {
		get_pixel_buffer[i] += get_pixel_buffer[i-1];
	}
	for (unsigned int i = total_pixels-1; i >0; --i) {
		get_pixel_buffer[i] = get_pixel_buffer[i-1];
	}
	get_pixel_buffer[0] = 0;
	// for (unsigned int i = total_pixels-1; i > 0; --i) {
	// 	get_pixel_buffer[i] = get_pixel_buffer[i-1];
	// }

	//printf("hmm: %i\n", get_pixel_buffer[total_pixels-1]);

	// set the offset buffer 
	glBindTexture(GL_TEXTURE_2D, head_pointer_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, MAX_FRAMEBUFFER_WIDTH, MAX_FRAMEBUFFER_HEIGHT, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, get_pixel_buffer);

	// clear atomic_counter_buffer
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, head_pointer_initializer);
	glBindTexture(GL_TEXTURE_2D, atomic_counter_array_buffer_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, MAX_FRAMEBUFFER_WIDTH, MAX_FRAMEBUFFER_HEIGHT, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);


	// render the scene
	gl4::ShaderManager::getInstance()->bindShader("oit_render");

    glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gl4::TextureManager::getInstance()->getTexture("checkerboard"));
	glUniform1i(3, 0);
	glUniformMatrix4fv(4, 1, GL_FALSE, &camera_transform[0][0]);
	render_scene();
	gl4::ShaderManager::getInstance()->unbindShader();

	
	
	// prevent sorting if S is pressed
	if(!engine->isKeyPressed('P')) {
		glUseProgram(sort_shader);
		glDispatchComputeIndirect(0);
		glUseProgram(0);
	}
	
	/*
	glUseProgram(render_shader);
	glDispatchComputeIndirect(0);
	glUseProgram(0);
	*/
/*
	glBindTexture(GL_TEXTURE_2D, dof_final);
	glGetTexImage(GL_TEXTURE_2D,0,GL_RED_INTEGER,GL_UNSIGNED_INT,get_dof_buffer);
	for (unsigned int i = 0; i < total_pixels*4; i+=4) {
		printf("(%f, %f, %f, %f)\n", get_dof_buffer[i+0], get_dof_buffer[i+1], get_dof_buffer[i+2], get_dof_buffer[i+3]);
	}
*/

	const float d_near = 0.0f;
	const float d_far = 30.0f;
	const int layers = 2;
	const float steps = (d_far - d_near) / layers;
	GLuint lastTarget = 0;

/*
	 {
		float from =0.0f;
		float to = 5.0f;

		glUseProgram(dof_render_shader);


		// render to temp
	    glBindImageTexture(3, fbo_tmp.getTexture(0), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glUniform1f(0, from);
		glUniform1f(1, to);
		glDispatchComputeIndirect(0);
		glFinish();
		glUseProgram(0);
	    lastTarget = fbo_tmp.getTexture(0);

	    render_to_final(lastTarget);

	    from =5.0f;
		to = 100.0f;

		glUseProgram(dof_render_shader);


		// render to temp
	    glBindImageTexture(3, fbo_tmp.getTexture(0), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glUniform1f(0, from);
		glUniform1f(1, to);
		glDispatchComputeIndirect(0);
		glFinish();
		glUseProgram(0);
	    lastTarget = fbo_tmp.getTexture(0);

	    for (int j = 0; j < 2; ++j)
	    {
	    	// blur to ping
			ping.bind();
			gl4::ShaderManager::getInstance()->bindShader("blur_x");
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, lastTarget);
			//glBindTexture(GL_TEXTURE_2D, gl4::TextureManager::getInstance()->getTexture("checkerboard"));
			glUniform1i(3, 0);
			engine->useOrthogonalProjection();
			obj->render();

			ping.unbind();

			// blur to pong
			pong.bind();
			gl4::ShaderManager::getInstance()->bindShader("blur_y");
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ping.getTexture(0));
			glUniform1i(3, 0);
			engine->useOrthogonalProjection();
			obj->render();

			pong.unbind();
			lastTarget = pong.getTexture(0);
	    }

		

		// combine
		//render_to_final(ping.getTexture(0));
		//render_to_final(pong.getTexture(0));
		//render_to_final(dof_temp);

		//ping.clear();
		//pong.clear();
	}
	*/
	
	for (int i = 0; i < layers; ++i) {
		float from = d_near + i*steps;// -1.0f;
		float to = d_near + (i+1)*steps;//+0.001f;

		glUseProgram(dof_render_shader);


		// render to temp
	    glBindImageTexture(3, fbo_tmp.getTexture(0), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glUniform1f(0, from);
		glUniform1f(1, to);
		glDispatchComputeIndirect(0);
		glFinish();
		glUseProgram(0);
	    lastTarget = fbo_tmp.getTexture(0);

	    for (int j = 0; j < i; ++j)
	    {
	    	// blur to ping
			ping.bind();
			gl4::ShaderManager::getInstance()->bindShader("blur_x");
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, lastTarget);
			//glBindTexture(GL_TEXTURE_2D, gl4::TextureManager::getInstance()->getTexture("checkerboard"));
			glUniform1i(3, 0);
			engine->useOrthogonalProjection();
			obj->render();

			ping.unbind();

			// blur to pong
			pong.bind();
			gl4::ShaderManager::getInstance()->bindShader("blur_y");
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ping.getTexture(0));
			glUniform1i(3, 0);
			engine->useOrthogonalProjection();
			obj->render();

			pong.unbind();
			lastTarget = pong.getTexture(0);
	    }

		

		// combine
		//render_to_final(ping.getTexture(0));
		render_to_final(lastTarget);
		//render_to_final(dof_temp);
		/*
		glUseProgram(dof_combine_shader);
	    glBindImageTexture(3, dof_final, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	    glBindImageTexture(4, pong.getTexture(0), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glDispatchComputeIndirect(0);
		glUseProgram(0);
		*/
		//ping.clear();
		//pong.clear();
	}
	

	
	// do the actual draw
	gl4::ShaderManager::getInstance()->bindShader("quad_texture");
	glBindImageTexture(4, fbo_final.getTexture(0), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	//glBindImageTexture(3, dof_temp, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	//glBindImageTexture(3,  fbo_tmp.getTexture(0), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	//glBindImageTexture(3,  ping.getTexture(0), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	//glBindImageTexture(3,  pong.getTexture(0), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	engine->useOrthogonalProjection();
	obj->render();
	gl4::ShaderManager::getInstance()->unbindShader();

	engine->glCheckError();
}

void render_scene() {
	glm::vec4 red(1.0,0.0,0.0,0.2);
	glm::vec4 blue(0.0,0.0,1.0,0.2);
	glm::vec4 green(0.0,1.0,0.0,0.2);

	float bunny_scale = 9.0;
	glm::vec4 bunny_color(1.0,1.0,0.0,0.2);
	glm::mat4 bunny_transform = glm::mat4(1.0);
	//bunny_transform = camera_transform *bunny_transform;
	bunny_transform = glm::translate(bunny_transform, glm::vec3( bunny_position[1]*0.05+10.0f, -0.3, -bunny_position[0]*0.05-5.0f));
	bunny_transform = glm::rotate(bunny_transform,bunny_position[2], glm::vec3(0.0f, 1.0f, 0.0f));
	bunny_transform = glm::rotate(bunny_transform,0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	bunny_transform = glm::scale(bunny_transform,glm::vec3(bunny_scale,bunny_scale,bunny_scale));

	float dragon_scale = 9.0;
	glm::vec4 dragon_color(0.0,1.0,1.0,0.2);
	glm::mat4 dragon_transform = glm::mat4(1.0);
	//dragon_transform = camera_transform *dragon_transform;
	dragon_transform = glm::translate(dragon_transform, glm::vec3(8.0f, -0.5, -3.0));
	dragon_transform = glm::rotate(dragon_transform,135.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	dragon_transform = glm::rotate(dragon_transform,0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	dragon_transform = glm::scale(dragon_transform,glm::vec3(dragon_scale,dragon_scale,dragon_scale));

	float armadillo_scale = 0.01; //armadillo_scale = 9.0;
	glm::vec4 armadillo_color(1.0,1.0,1.0,1.0);
	glm::mat4 armadillo_transform = glm::mat4(1.0);
	//armadillo_transform = camera_transform *armadillo_transform;
	armadillo_transform = glm::translate(armadillo_transform, glm::vec3(13.4f, 0.6, -6.2));
	armadillo_transform = glm::rotate(armadillo_transform,130.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	armadillo_transform = glm::rotate(armadillo_transform,0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	armadillo_transform = glm::scale(armadillo_transform,glm::vec3(armadillo_scale,armadillo_scale,armadillo_scale));

	float box_scale = 0.01;
	glm::mat4 box_transform = glm::mat4(1.0);
	//box_transform = camera_transform *box_transform;
	box_transform = glm::translate(box_transform, glm::vec3(13.0f, 1.0, -7.0));
	box_transform = glm::rotate(box_transform,40.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	box_transform = glm::rotate(box_transform,0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	box_transform = glm::scale(box_transform,glm::vec3(box_scale,box_scale,box_scale));

	float plane_scale = 20.0;
	glm::vec4 plane_color(1.0,1.0,1.0,0.0);

	glm::mat4 floor_transform = glm::mat4(1.0);
	glm::mat4 wall_transform = glm::mat4(1.0);
	//floor_transform = camera_transform *floor_transform;
	//wall_transform = camera_transform *wall_transform;
	floor_transform = glm::translate(floor_transform, glm::vec3(0.0, 0.0, 0.0));
	floor_transform = glm::rotate(floor_transform,-90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	floor_transform = glm::scale(floor_transform,glm::vec3(plane_scale,plane_scale,plane_scale));
	wall_transform = glm::translate(wall_transform, glm::vec3(0.0, 0.0, -20.0));
	wall_transform = glm::rotate(wall_transform,0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	wall_transform = glm::scale(wall_transform,glm::vec3(plane_scale,plane_scale,plane_scale));


    // WALL
	engine->usePerspectiveProjection(floor_transform);
    glUniform4fv(loc_base_color, 1, &plane_color[0]);
	plane->render();
	engine->usePerspectiveProjection(wall_transform);
	plane->render();

	// BUNNY
	engine->usePerspectiveProjection(bunny_transform);
    glUniform4fv(loc_base_color, 1, &bunny_color[0]);
	bunny->render();

	// dragon
	engine->usePerspectiveProjection(dragon_transform);
    glUniform4fv(loc_base_color, 1, &dragon_color[0]);
	dragon->render();

	// armadillo
	engine->usePerspectiveProjection(armadillo_transform);
    glUniform4fv(loc_base_color, 1, &armadillo_color[0]);
	armadillo->render();
	//bunny->render();


	// BOX
	engine->usePerspectiveProjection(box_transform);
    glUniform4fv(loc_base_color, 1, &red[0]);
	box->render();
}


void myUpdateFunc(float dt)
{
	float speed = 5.0f;

	camera_transform = glm::mat4(1.0);
	if(engine->isKeyPressed('W')) {
		camera_position[0] -= dt*speed;
	}
	if(engine->isKeyPressed('S')) {
		camera_position[0] += dt*speed;
	}
	if(engine->isKeyPressed('A')) {
		camera_position[1] -= dt*speed;
	}
	if(engine->isKeyPressed('D')) {
		camera_position[1] += dt*speed;
	}
	if(engine->isKeyPressed('R')) {
		camera_position[2] += dt*speed;
	}
	if(engine->isKeyPressed('F')) {
		camera_position[2] -= dt*speed;
	}
	if(engine->isKeyPressed('Q')) {
		camera_rotation[0] += dt*speed*7.0f;
	}
	if(engine->isKeyPressed('E')) {
		camera_rotation[0] -= dt*speed*7.0f;
	}
	if(engine->isKeyPressed('X')) {
		camera_rotation[1] += dt*speed*7.0f;
	}
	if(engine->isKeyPressed('C')) {
		camera_rotation[1] -= dt*speed*7.0f;
	}
	camera_transform = glm::translate(camera_transform, glm::vec3(camera_position[1], camera_position[2], camera_position[0]));
	camera_transform = glm::rotate(camera_transform,camera_rotation[0], glm::vec3(1.0f, 0.0f, 0.0f));
	camera_transform = glm::rotate(camera_transform,camera_rotation[1], glm::vec3(0.0f, 1.0f, 0.0f));
	camera_transform = glm::inverse(camera_transform);
	/*
	if(engine->isKeyPressed(GLFW_KEY_RIGHT)) {
		angle[1] += dt*speed;
	}
	if(engine->isKeyPressed(GLFW_KEY_LEFT)) {
		angle[1] -= dt*speed;
	}
	if(engine->isKeyPressed(GLFW_KEY_UP)) {
		angle[0] += dt*speed;
		//printf("angle: %f\n", angle[0]);
	}
	if(engine->isKeyPressed(GLFW_KEY_DOWN)) {
		angle[0] -= dt*speed;
	}
*/

	speed = 50.0f;
	if(engine->isKeyPressed('M')) {
		bunny_position[2] += dt*speed;
	}
	if(engine->isKeyPressed('N')) {
		bunny_position[2] -= dt*speed;
	}
	if(engine->isKeyPressed(GLFW_KEY_RIGHT)) {
		bunny_position[1] += dt*speed;
	}
	if(engine->isKeyPressed(GLFW_KEY_LEFT)) {
		bunny_position[1] -= dt*speed;
	}
	if(engine->isKeyPressed(GLFW_KEY_UP)) {
		bunny_position[0] += dt*speed;
		//printf("angle: %f\n", angle[0]);
	}
	if(engine->isKeyPressed(GLFW_KEY_DOWN)) {
		bunny_position[0] -= dt*speed;
	}	
}

void render_to_final(GLuint texture) {

	glEnable(GL_BLEND);
	//glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_ONE);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	//glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
	//glBlendFunc(GL_ZERO, GL_ONE);
	//glBlendFunc(GL_ONE, GL_ZERO);
	glBlendFunc(GL_ONE, GL_ONE);
	//glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
	fbo_final.bind();
	gl4::ShaderManager::getInstance()->bindShader("quad_texture");
	
	glBindImageTexture(4, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	/*
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(3, 0);
	*/
	engine->useOrthogonalProjection();
	obj->render();
	gl4::ShaderManager::getInstance()->unbindShader();
	fbo_final.unbind();
	glBlendFunc(GL_ONE, GL_ZERO);
	glDisable(GL_BLEND);
}

