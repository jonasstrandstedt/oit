#ifndef OIT_FIXED
#define OIT_FIXED

#include "Engine.h"

class oit_fixed {

public:
	oit_fixed(int w, int h, int s);
	~oit_fixed();

	void render(const GLfloat *camera_transform, GLuint out_texture, void (*f)(void));

private:

	GLuint max_width, max_height, max_samples;

	GLuint *data;
	GLfloat *dataf;
	size_t total_pixels;
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

};

#endif