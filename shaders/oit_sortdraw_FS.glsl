/**
Copyright (C) 2012-2014 Jonas Strandstedt

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#version 430

layout (binding = 0, r32ui) uniform uimage2D head_pointer_image;
layout (binding = 1, rgba32ui) uniform uimageBuffer list_buffer;

#define MAX_FRAGMENTS 256

uvec4 fragments[MAX_FRAGMENTS];

layout (location = 0) out vec4 diffuse;

int build_local_fragments_list() {
	uint current;
	int frag_count = 0;

	current = imageLoad(head_pointer_image, ivec2(gl_FragCoord.xy)).x;

	while(current != 0 && frag_count < MAX_FRAGMENTS) {
		uvec4 item = imageLoad(list_buffer, int(current));
		current = item.x;

		fragments[frag_count] = item;

		frag_count++;
	}

	return frag_count;
}

void sort_fragments_list(int frag_count) {
	int i;
	int j;

	for(i = 0; i < frag_count; i++) {
		for(j = i + 1; j < frag_count; j++) {
			float depth_i = uintBitsToFloat(fragments[i].z);
			float depth_j = uintBitsToFloat(fragments[j].z);

			if(depth_i > depth_j) {
				uvec4 tmp = fragments[i];
				fragments[i] = fragments[j];
				fragments[j] = tmp;
			}
		}
	}
}

vec4 blend(vec4 current_color, vec4 new_color) {
	return mix(current_color, new_color, new_color.a);
}

vec4 calculate_final_color(int frag_count) {
	vec4 final_color = vec4(0);

	/*
	int i;
	for(i = 0; i < frag_count; i++) {
		vec4 frag_color = unpackUnorm4x8(fragments[i].y);
		final_color = blend(final_color, frag_color);
	}
	*/
	uint current = imageLoad(head_pointer_image, ivec2(gl_FragCoord.xy)).x;
	while(current != 0) {
		uvec4 item = imageLoad(list_buffer, int(current-1));
		vec4 frag_color = unpackUnorm4x8(item.y);
		final_color = blend(final_color, frag_color);
		current = item.x;
	}

	return final_color;
}

void main()
{
	int frag_count = 0;

	//frag_count = build_local_fragments_list();

	//sort_fragments_list(frag_count);

	diffuse = calculate_final_color(frag_count);
}