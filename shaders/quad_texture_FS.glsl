/**
Copyright (C) 2012-2014 Jonas Strandstedt

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#version 430


//layout (location = 3) uniform sampler2DArray sampler_out_texture;

layout (binding = 0, r32ui) uniform uimage2D head_pointer_image;
layout (binding = 1, rgba32ui) uniform uimageBuffer list_buffer;
layout (binding = 2, r32ui) uniform uimage2D atomic_counter_array_buffer_texture;

//layout (binding = 3, rgba32f) uniform image2DArray out_texture;
//readonly layout (binding = 3, size4x32) uniform image2DArray out_texture;
layout (binding = 3, rgba32f) uniform image2D out_texture;



layout (location = 0) out vec4 diffuse;


void main()
{

	
	//diffuse = texelFetch(sampler_out_texture, ivec3(gl_FragCoord.xy,0),0);
	//diffuse = texelFetch(sampler_out_texture, ivec2(gl_FragCoord.xy),0);
	//diffuse = texture(sampler_out_texture,vec3(gl_FragCoord.xy,0));
	//diffuse = vec4(1,1,1,1);

	//diffuse = imageLoad(out_texture, ivec3(gl_FragCoord.xy,0));
	diffuse = imageLoad(out_texture, ivec2(gl_FragCoord.xy));
}