/**
Copyright (C) 2012-2014 Jonas Strandstedt

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#version 430


layout(location = 0) in vec2 in_tex;
layout(location = 1) in vec3 in_position;
layout(location = 3) uniform sampler2D image;

layout (location = 0) out vec4 diffuse;

uniform float offset[3] = float[]( 0.0, 1.3846153846, 3.2307692308 );
uniform float weight[3] = float[]( 0.2270270270, 0.3162162162, 0.0702702703 );

#define WIDTH 800.0

void main()
{
	diffuse = texture( image, vec2(in_tex) ) * weight[0];
	
	for (int i=1; i<3; i++) {
		diffuse += texture( image, ( in_tex+vec2(offset[i],0.0)/WIDTH ) ) * weight[i];
		diffuse += texture( image, ( in_tex-vec2(offset[i],0.0)/WIDTH ) ) * weight[i];
	}
		

/*
	vec4 sum;

	int count = 4;
	vec2 diff = vec2(1.0,0.0);

	sum += texture(image, vec2(gl_FragCoord.xy - diff*4)) * 1.0;
	sum += texture(image, vec2(gl_FragCoord.xy - diff*3)) * 2.0;
	sum += texture(image, vec2(gl_FragCoord.xy - diff*2)) * 4.0;
	sum += texture(image, vec2(gl_FragCoord.xy + diff*1)) * 8.0;
	sum += texture(image, vec2(gl_FragCoord.xy + diff*0)) * 16.0;
	sum += texture(image, vec2(gl_FragCoord.xy + diff*1)) * 8.0;
	sum += texture(image, vec2(gl_FragCoord.xy + diff*2)) * 4.0;
	sum += texture(image, vec2(gl_FragCoord.xy + diff*3)) * 2.0;
	sum += texture(image, vec2(gl_FragCoord.xy + diff*4)) * 1.0;

	sum = sum / 46.0;

	diffuse = sum;
*/
	/*

	vec4 sum;

	int count = 4;
	float kernel_size = 0.0;
	vec2 diff = vec2(1.0,0.0);

	sum += imageLoad(in_image, ivec2(gl_FragCoord.xy - diff*4)) * 1.0;
	sum += imageLoad(in_image, ivec2(gl_FragCoord.xy - diff*3)) * 2.0;
	sum += imageLoad(in_image, ivec2(gl_FragCoord.xy - diff*2)) * 4.0;
	sum += imageLoad(in_image, ivec2(gl_FragCoord.xy + diff*1)) * 8.0;
	sum += imageLoad(in_image, ivec2(gl_FragCoord.xy + diff*0)) * 16.0;
	sum += imageLoad(in_image, ivec2(gl_FragCoord.xy + diff*1)) * 8.0;
	sum += imageLoad(in_image, ivec2(gl_FragCoord.xy + diff*2)) * 4.0;
	sum += imageLoad(in_image, ivec2(gl_FragCoord.xy + diff*3)) * 2.0;
	sum += imageLoad(in_image, ivec2(gl_FragCoord.xy + diff*4)) * 1.0;

	sum = sum / 46.0;

	diffuse = sum;	
	*/
}