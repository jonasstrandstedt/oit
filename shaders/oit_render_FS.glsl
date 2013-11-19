/**
Copyright (C) 2012-2014 Jonas Strandstedt

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#version 430

layout (early_fragment_tests) in;

layout(location = 0) uniform mat4 Projection;
layout(location = 1) uniform mat4 ModelTransform;

layout (binding = 0, r32ui) uniform uimage2D head_pointer_image;
layout (binding = 1, rgba32ui) uniform uimageBuffer list_buffer;

layout (binding = 0, offset = 0) uniform atomic_uint index_counter;
layout (location = 0) out vec4 diffuse;

layout (location = 0) in vec2 st;
layout (location = 1) in vec3 frag_position;
layout (location = 2) in vec3 frag_normal;

uniform vec3 light_position = vec3(40.0, 20.0, 100.0);

void main()
{

	vec4 frag_color = vec4(1.0,0,0,0.5);
	uint index = atomicCounterIncrement(index_counter);
	uint old_head = imageAtomicExchange(head_pointer_image, ivec2(gl_FragCoord.xy), index);

	vec3 L = normalize(light_position - frag_position);
    vec3 V = normalize(-frag_position);
    vec3 N = normalize(frag_normal);
    vec3 H = normalize(L + V);

    float NdotL = dot(N, L);
    float NdotH = dot(N, H);

    frag_color = vec4(frag_color.rgb * abs(NdotL), frag_color.a);

	uvec4 item;
	item.x = old_head;
	item.y = packUnorm4x8(frag_color);
	item.z = floatBitsToUint(gl_FragCoord.z);
	item.w = 0;

	imageStore(list_buffer, int(index), item);
	diffuse = frag_color;

}