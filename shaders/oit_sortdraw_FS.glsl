/**
Copyright (C) 2012-2014 Jonas Strandstedt

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#version 430

layout (binding = 0, r32ui) uniform uimage2D head_pointer_image;
layout (binding = 1, rgba32ui) uniform uimageBuffer list_buffer;
layout (binding = 2, r32ui) uniform uimage2D atomic_counter_array_buffer_texture;

#define MAX_FRAGMENTS 32

uvec4 fragments[MAX_FRAGMENTS];

layout (location = 0) out vec4 diffuse;

void build_local_fragments_list(uint offset, uint frag_count) {
	uint current;

	uint i;
	for(i = 0; i < frag_count && i < MAX_FRAGMENTS; i++) {
		uvec4 item = imageLoad(list_buffer, int(offset+i));
		fragments[i] = item;
	}

}

void sort_fragments_list(uint frag_count) {
	uint i,j;
	uvec4 tmp;

	/*
	// BUBBLE SORT
	for(i = 0; i < frag_count; i++) {
		for(j = i + 1; j < frag_count; j++) {
			if(fragments[i].z <fragments[j].z) {
				tmp = fragments[i];
				fragments[i] = fragments[j];
				fragments[j] = tmp;
			}
		}
	}
	*/

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


vec4 calculate_final_color(uint frag_count) {
	
	vec4 final_color = vec4(0);
	for(uint i = 0; i < frag_count; i++) {
		
		uvec4 item = fragments[i];
		vec4 frag_color = unpackUnorm4x8(item.y);
		final_color = blend(final_color, frag_color);
	}

	return final_color;

}

/*
vec4 calculate_final_color(uint offset, uint frag_count) {


	vec4 final_color = vec4(0);
	for(uint i = 0; i < frag_count; i++) {
		
		uvec4 item = imageLoad(list_buffer, int(offset+i));;
		vec4 frag_color = unpackUnorm4x8(item.y);
		final_color = blend(final_color, frag_color);
	}

	return final_color;
}
*/

void main()
{
	uint offset = imageLoad(head_pointer_image, ivec2(gl_FragCoord.xy));
	uint frag_count = imageLoad(atomic_counter_array_buffer_texture, ivec2(gl_FragCoord.xy)).x;

	build_local_fragments_list(offset, frag_count);
	sort_fragments_list(frag_count);
	diffuse = calculate_final_color(frag_count);
	//diffuse = calculate_final_color(offset,frag_count);

	//diffuse = vec4(0, imageLoad(atomic_counter_array_buffer_texture, ivec2(gl_FragCoord.xy)).x / 6.0f, 0 ,1);
}