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
layout(location = 2) uniform vec4 base_color;
layout(location = 3) uniform sampler2D base_texture;

layout (location = 0) in vec2 st;
layout (location = 1) in vec3 frag_position;
layout (location = 2) in vec3 frag_normal;

uniform vec3 light_position = vec3(40.0, 20.0, 100.0);

layout(location = 0) out vec4 out_color;

void main()
{

	vec4 frag_color = base_color;
	if(base_color.a == 0.0) {
		frag_color = texture(base_texture, st);
	}

	vec3 L = normalize(light_position - frag_position);
    vec3 V = normalize(-frag_position);
    vec3 N = normalize(frag_normal);
    vec3 H = normalize(L + V);

    float NdotL = dot(N, L);
    float NdotH = dot(N, H);

    frag_color = vec4(frag_color.rgb * abs(NdotL), frag_color.a);

    out_color = frag_color;
}