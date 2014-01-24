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

#include "oit_linked.h"
#include "oit_fixed.h"
#include "oit_dynamic.h"


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

GLuint oit_version = 0;

GLuint loc_base_color;
oit_linked * oit1;
oit_dynamic * oit2;
oit_fixed * oit3;
gl4::FBO fbo_final;


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

	// return success
	return 0;
}

void keyboardCallback(int key, int state) 
{
	if(key == '1' && state == GLFW_PRESS) {
		oit_version = 1;
	}
	if(key == '2' && state == GLFW_PRESS) {
		oit_version = 2;
	}
	if(key == '3' && state == GLFW_PRESS) {
		oit_version = 3;
	}
	if(key == '0' && state == GLFW_PRESS) {
		oit_version = 0;
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
	obj->setProportions(MAX_FRAMEBUFFER_WIDTH, MAX_FRAMEBUFFER_HEIGHT);
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

	gl4::Shader *quad_texture = new gl4::Shader("../shaders/quad_texture_VS.glsl", "../shaders/quad_texture_FS.glsl");
	gl4::ShaderManager::getInstance()->addShaderProgram("quad_texture", quad_texture);
	gl4::Shader *passthrough = new gl4::Shader("../shaders/passthrough_VS.glsl", "../shaders/passthrough_FS.glsl");
	gl4::ShaderManager::getInstance()->addShaderProgram("passthrough", passthrough);

	oit1 = new oit_linked(MAX_FRAMEBUFFER_WIDTH, MAX_FRAMEBUFFER_HEIGHT, MAX_FRAMEBUFFER_SAMPLES);
	oit2 = new oit_dynamic(MAX_FRAMEBUFFER_WIDTH, MAX_FRAMEBUFFER_HEIGHT, MAX_FRAMEBUFFER_SAMPLES);
	oit3 = new oit_fixed(MAX_FRAMEBUFFER_WIDTH, MAX_FRAMEBUFFER_HEIGHT, MAX_FRAMEBUFFER_SAMPLES);

	fbo_final.setTextureFormat(GL_RGBA32F);
	fbo_final.init(MAX_FRAMEBUFFER_WIDTH,MAX_FRAMEBUFFER_HEIGHT);


	engine->glCheckError();
    printf("Init done!\n");

}

void myRenderFunc(void) 
{
	

	fbo_final.clear();

	switch(oit_version) {
		case 1:
			oit1->render(&camera_transform[0][0], fbo_final.getTexture(0), render_scene);
			break;
		case 2:
			oit2->render(&camera_transform[0][0], fbo_final.getTexture(0), render_scene);
			break;
		case 3:
			oit3->render(&camera_transform[0][0], fbo_final.getTexture(0), render_scene);
			break;
		default:
			fbo_final.bind();
			gl4::ShaderManager::getInstance()->bindShader("passthrough");
			glUniformMatrix4fv(4, 1, GL_FALSE, &camera_transform[0][0]);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gl4::TextureManager::getInstance()->getTexture("checkerboard"));
			glUniform1i(3, 0);
			render_scene();
			fbo_final.unbind();
			break;
	}
	
	// do the actual draw
	gl4::ShaderManager::getInstance()->bindShader("quad_texture");
	glBindImageTexture(4, fbo_final.getTexture(0), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	engine->useOrthogonalProjection();
	obj->render();
	gl4::ShaderManager::getInstance()->unbindShader();

	engine->glCheckError();
}

void render_scene() {
	glm::vec4 red(1.0,0.0,0.0,0.2);
	glm::vec4 blue(0.0,0.0,1.0,0.2);
	glm::vec4 green(0.0,1.0,0.0,0.2);
	loc_base_color = 2;

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

