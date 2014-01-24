#ifndef OIT_LINKED
#define OIT_LINKED

#include "Engine.h"

class oit_linked {

public:
	oit_linked(int w, int h, int s);
	~oit_linked();

	void render(const GLfloat *camera_transform, GLuint out_texture, void (*f)(void));

private:

	GLuint max_width, max_height, max_samples;

	GLuint *data;
	size_t total_pixels;
	GLuint head_pointer_texture;
	GLuint head_pointer_initializer;
	GLuint atomic_counter_buffer;
	GLuint fragment_storage_buffer;
	GLuint linked_list_texture;
	GLuint sort_shader;
	GLuint render_shader;
	GLuint dispatch_buffer;

};

#endif