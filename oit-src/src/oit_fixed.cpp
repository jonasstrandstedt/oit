
#include "oit_fixed.h"

const GLchar * fixed_render_source [] = {
R"(#version 430

#define MAX_FRAGMENTS_LAYERS 16

layout(local_size_x = 16, local_size_y = 16) in;

layout (binding = 0, r32ui) uniform uimage2D head_pointer_image;
layout (binding = 1, rgba32ui) uniform uimageBuffer list_buffer;
layout (binding = 2, r32ui) uniform uimage2D atomic_counter_array_buffer_texture;
layout (binding = 3, rgba32f) writeonly uniform image2D out_texture;
uvec4 fragments[MAX_FRAGMENTS_LAYERS];

void build_local_fragments_list(uint offset, uint frag_count) {
	uint current;

	uint i;
	for(i = 0; i < frag_count && i < MAX_FRAGMENTS_LAYERS; i++) {
		uvec4 item = imageLoad(list_buffer, int(offset+i));
		fragments[i] = item;
	}

}

void sort_fragments_list(uint frag_count) {
	uint i,j;
	uvec4 tmp;

	// INSERTION SORT
	for(i = 1; i < frag_count; ++i) {
		tmp = fragments[i];
		for(j = i; j > 0 && tmp.z > fragments[j-1].z; --j) {
			fragments[j] = fragments[j-1];
		}
		fragments[j] = tmp;
	}
}

vec4 blend(vec4 current_color, vec4 new_color) {
	return mix(current_color, new_color, new_color.a);
}

vec4 calculate_final_color(uint offset, uint frag_count) {
	
	vec4 final_color = vec4(0);
	for(uint i = 0; i < frag_count; i++) {
		
		//uvec4 item = imageLoad(list_buffer, int(offset+i));
		uvec4 item = fragments[i];
		vec4 frag_color = unpackUnorm4x8(item.y);
		final_color = blend(final_color, frag_color);
	}

	return final_color;

}

void main()
{
	uint offset;// = imageLoad(head_pointer_image, ivec2(gl_GlobalInvocationID.xy));
	offset = (uint(gl_GlobalInvocationID.y) * 800 + uint(gl_GlobalInvocationID.x))*10;
	uint frag_count = imageLoad(atomic_counter_array_buffer_texture, ivec2(gl_GlobalInvocationID.xy)).x;

	build_local_fragments_list(offset, frag_count);
	sort_fragments_list(frag_count);
	vec4 diffuse = calculate_final_color(offset,frag_count);


	//imageStore(out_texture, ivec3(gl_GlobalInvocationID.xy,0), diffuse);
	imageStore(out_texture, ivec2(gl_GlobalInvocationID.xy), diffuse);
}

)"};

oit_fixed::oit_fixed(int w, int h, int s) {

	max_width = w;
	max_height = h;
	max_samples = s;

	total_pixels = max_width * max_height;

	glGenTextures(1, &head_pointer_texture);
	glBindTexture(GL_TEXTURE_2D, head_pointer_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, max_width, max_height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);

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
	glBufferData(GL_TEXTURE_BUFFER, max_samples*total_pixels*sizeof(GLfloat)*4, NULL, GL_DYNAMIC_COPY);

    glGenTextures(1, &linked_list_texture);
    glBindTexture(GL_TEXTURE_BUFFER, linked_list_texture);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32UI, fragment_storage_buffer);
    glBindTexture(GL_TEXTURE_BUFFER, 0);

    glBindImageTexture(1, linked_list_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32UI);


   // COMPUTE SHADERS
	char str[4096];
	GLint shadersLinked = GL_FALSE;
    static const struct
	{
		GLuint num_groups_x;
		GLuint num_groups_y;
		GLuint num_groups_z;
	} dispatch_params = { max_width / 16, max_height / 16, 1};

	// RENDER SHADER
	GLuint render_shader_part = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(render_shader_part, 1, fixed_render_source, NULL);
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


	gl4::Shader *oit_render = new gl4::Shader("../shaders/oit_fixed_render_VS.glsl", "../shaders/oit_fixed_render_FS.glsl");
	gl4::ShaderManager::getInstance()->addShaderProgram("oit_fixed_render", oit_render);

}

oit_fixed::~oit_fixed() {

}

void oit_fixed::render(const GLfloat *camera_transform, GLuint out_texture, void (*f)(void)) {

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, head_pointer_initializer);

	// clear head_pointer
	glBindTexture(GL_TEXTURE_2D, head_pointer_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, max_width, max_height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);

	// clear atomic_counter_array_buffer
	glBindTexture(GL_TEXTURE_2D, atomic_counter_array_buffer_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, max_width, max_height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	// Bind head-pointer image for read-write
    glBindImageTexture(0, head_pointer_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(1, linked_list_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32UI);
    glBindImageTexture(2, atomic_counter_array_buffer_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
	glBindImageTexture(3, out_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	gl4::ShaderManager::getInstance()->bindShader("oit_fixed_render");

    glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gl4::TextureManager::getInstance()->getTexture("checkerboard"));
	glUniform1i(3, 0);
	glUniformMatrix4fv(4, 1, GL_FALSE, camera_transform);
	f();
	gl4::ShaderManager::getInstance()->unbindShader();


	glUseProgram(render_shader);
	glDispatchComputeIndirect(0);
	glUseProgram(0);
	
}