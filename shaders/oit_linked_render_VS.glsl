/**
Copyright (C) 2012-2014 Jonas Strandstedt

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#version 430

layout(location = 0) uniform mat4 Projection;
layout(location = 1) uniform mat4 ModelTransform;
layout(location = 2) uniform vec4 base_color;
layout(location = 3) uniform sampler2D base_texture;
layout(location = 4) uniform mat4 CameraTransform;

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec2 vertex_tex;
layout(location = 2) in vec3 vertex_normal;
layout(location = 3) in vec4 vertex_color;
layout(location = 4) in vec3 vertex_attribute3f;
layout(location = 5) in float vertex_attribute1f;

layout(location = 0) out vec2 out_tex;
layout(location = 1) out vec3 out_position;
layout(location = 2) out vec3 out_normal;

void main()
{
	vec4 vpos = ModelTransform  * vec4(vertex_position,1);
	vec4 vnorm =  ModelTransform * vec4(vertex_normal,0);

	out_tex = vertex_tex;
	out_position =  vpos.xyz;
	out_normal =  vnorm.xyz;

	gl_Position = Projection * CameraTransform* vpos;
} 