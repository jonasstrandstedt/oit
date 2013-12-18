
#ifndef MAIN_H
#define MAIN_H

const GLchar * sort_source [] = {
R"(#version 430

layout(local_size_x = 16, local_size_y = 16) in;

layout (binding = 0, r32ui) uniform uimage2D head_pointer_image;
layout (binding = 1, rgba32ui) uniform uimageBuffer list_buffer;
layout (binding = 2, r32ui) uniform uimage2D atomic_counter_array_buffer_texture;

void main() {

	uint offset = imageLoad(head_pointer_image, ivec2(gl_GlobalInvocationID.xy));
	uint frag_count = imageLoad(atomic_counter_array_buffer_texture, ivec2(gl_GlobalInvocationID.xy)).x;
	int frag_counti = int(frag_count);

	uvec4 item_i;
	uvec4 item_j;

	float depth_i;
	float depth_j;

	
/*
	// SELECTION SORT
	uvec4 item_min, item_next;
	int iMin;
	float depth_min;
	item_next = imageLoad(list_buffer, int(offset));
	depth_min = uintBitsToFloat(item_next.z);

	for(int i = 0; i < frag_counti-1; i++) {
		iMin = i;
		item_i = item_next;
		item_min = item_i;

		for(int j = i+1; j < frag_counti; j++) {
			item_j = imageLoad(list_buffer, int(offset + j));

			if(i+1 == j) {
				item_next = item_j;
			}

			if(item_i.z < item_j.z) {
				item_min = item_j;
				iMin = j;
			}
		}

		// something was smaller, swap
		if (iMin > i)
		{
			imageStore(list_buffer, int(offset + i), item_min);
			imageStore(list_buffer, int(offset + iMin), item_i);
		}
		
	}
*/


	// BUBBLE SORT
	uvec4 tmp;
	for(int i = 0; i < frag_counti-1; i++) {
		item_i = imageLoad(list_buffer, int(offset + i));

		for(int j = i+1; j < frag_counti; j++) {
			item_j = imageLoad(list_buffer, int(offset + j));

			if(item_i.z < item_j.z) {
				imageStore(list_buffer, int(offset + i), item_j);
				imageStore(list_buffer, int(offset + j), item_i);

				tmp = item_i;
				item_i = item_j;
				item_j = tmp;

			}
		}
		
	}



};

)"};

const GLchar * render_source [] = {
R"(#version 430

#define MAX_FRAGMENTS_LAYERS 2

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
		
		uvec4 item = imageLoad(list_buffer, int(offset+i));
		vec4 frag_color = unpackUnorm4x8(item.y);
		final_color = blend(final_color, frag_color);
	}

	return final_color;

}

void main()
{
	uint offset = imageLoad(head_pointer_image, ivec2(gl_GlobalInvocationID.xy));
	uint frag_count = imageLoad(atomic_counter_array_buffer_texture, ivec2(gl_GlobalInvocationID.xy)).x;

	//build_local_fragments_list(offset, frag_count);
	//sort_fragments_list(frag_count);
	vec4 diffuse = calculate_final_color(offset,frag_count);


	//imageStore(out_texture, ivec3(gl_GlobalInvocationID.xy,0), diffuse);
	imageStore(out_texture, ivec2(gl_GlobalInvocationID.xy), diffuse);
}

)"};

const GLchar * render_dof_source [] = {
R"(#version 430

#define MAX_FRAGMENTS_LAYERS 2

layout(local_size_x = 16, local_size_y = 16) in;

layout (location = 0) uniform float depth_min;
layout (location = 1) uniform float depth_max;

layout (binding = 0, r32ui) uniform uimage2D head_pointer_image;
layout (binding = 1, rgba32ui) uniform uimageBuffer list_buffer;
layout (binding = 2, r32ui) uniform uimage2D atomic_counter_array_buffer_texture;
layout (binding = 3, rgba32f) writeonly uniform image2D out_texture;

vec4 blend(vec4 current_color, vec4 new_color) {
	return mix(current_color, new_color, new_color.a);
}

vec4 calculate_final_color(uint offset, uint frag_count) {
	
	vec4 final_color = vec4(0);
	for(uint i = 0; i < frag_count; i++) {
		
		uvec4 item = imageLoad(list_buffer, int(offset+i));
		float depth = uintBitsToFloat(item.z);

		if(depth >= depth_min && depth <= depth_max) {
			final_color = blend(final_color, unpackUnorm4x8(item.y));
		}
	}

	return final_color;

}

void main()
{
	uint offset = imageLoad(head_pointer_image, ivec2(gl_GlobalInvocationID.xy));
	uint frag_count = imageLoad(atomic_counter_array_buffer_texture, ivec2(gl_GlobalInvocationID.xy)).x;

	vec4 diffuse = calculate_final_color(offset,frag_count);
	imageStore(out_texture, ivec2(gl_GlobalInvocationID.xy), diffuse);
}

)"};

const GLchar * combine_dof_source [] = {
R"(#version 430

#define MAX_FRAGMENTS_LAYERS 2

layout(local_size_x = 16, local_size_y = 16) in;

layout (binding = 3, rgba32f) uniform image2D final_texture;
layout (binding = 4, rgba32f) uniform image2D in_texture;

vec4 blend(vec4 current_color, vec4 new_color) {
	
	vec4 result;
	result.rgb = current_color.rgb + (1.0 - current_color.a) * new_color.a * new_color.rgb;
    result.a = current_color.a + (1.0 -current_color.a) * new_color.a;
    return result;
	//return mix(current_color, new_color, new_color.a);
}

void main()
{
	vec4 diffuse_final = imageLoad(final_texture, ivec2(gl_GlobalInvocationID.xy));
	vec4 diffuse_in = imageLoad(in_texture, ivec2(gl_GlobalInvocationID.xy));


	/*

	if (diffuse_final.a > 0.0)
	{
		imageStore(final_texture, ivec2(gl_GlobalInvocationID.xy), blend(diffuse_final, diffuse_in));
	} else {
		imageStore(final_texture, ivec2(gl_GlobalInvocationID.xy), diffuse_in);

	}
	*/


	
	vec4 diffuse_out = blend(diffuse_final, diffuse_in);
	imageStore(final_texture, ivec2(gl_GlobalInvocationID.xy), diffuse_out);
	//imageStore(final_texture, ivec2(gl_GlobalInvocationID.xy), diffuse_out);
}

)"};



#endif