/**
Copyright (C) 2012-2014 Jonas Strandstedt

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <GL/glfw.h>
//#include "glext.h"
//#include "glcorearb.h"

#include <ostream>

/*
#define GL_COMPUTE_SHADER                 0x91B9
#define GL_MAX_COMPUTE_UNIFORM_BLOCKS     0x91BB
#define GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS 0x91BC
#define GL_MAX_COMPUTE_IMAGE_UNIFORMS     0x91BD
#define GL_MAX_COMPUTE_SHARED_MEMORY_SIZE 0x8262
#define GL_MAX_COMPUTE_UNIFORM_COMPONENTS 0x8263
#define GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS 0x8264
#define GL_MAX_COMPUTE_ATOMIC_COUNTERS    0x8265
#define GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS 0x8266
#define GL_MAX_COMPUTE_LOCAL_INVOCATIONS  0x90EB
#define GL_MAX_COMPUTE_WORK_GROUP_COUNT   0x91BE
#define GL_MAX_COMPUTE_WORK_GROUP_SIZE    0x91BF
#define GL_COMPUTE_LOCAL_WORK_SIZE        0x8267
#define GL_UNIFORM_BLOCK_REFERENCED_BY_COMPUTE_SHADER 0x90EC
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_COMPUTE_SHADER 0x90ED
#define GL_COMPUTE_TEXTURE                0x82A0
#define GL_COMPUTE_SUBROUTINE             0x92ED
#define GL_COMPUTE_SUBROUTINE_UNIFORM     0x92F3
#define GL_REFERENCED_BY_COMPUTE_SHADER   0x930B
#define GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS 0x90DB
#define GL_COMPUTE_SHADER_BIT             0x00000020
#define GL_COMPUTE_PROGRAM_NV             0x90FB
#define GL_COMPUTE_PROGRAM_PARAMETER_BUFFER_NV 0x90FC
#define GL_PATH_COMPUTED_LENGTH_NV        0x90A0
#define GL_DISPATCH_INDIRECT_BUFFER       0x90EE
#define GL_DISPATCH_INDIRECT_BUFFER_BINDING 0x90EF
*/

enum {
	UNIFORM_PROJECTION = 0,
	UNIFORM_MODELTRANSFORM,
	UNIFORM_TEXTURE0,
	UNIFORM_TEXTURE1,
	UNIFORM_TEXTURE2,
	UNIFORM_TEXTURE3,
	UNIFORM_TEXTURE4,
	UNIFORM_TEXTURE5,
	UNIFORM_TEXTURE6,
	UNIFORM_TEXTURE7,
	UNIFORM_TEXTURE8,
	UNIFORM_TESSLEVEL,
	UNIFORM_COLOR,
	UNIFORM_WINDOWSIZE,
	UNIFORM_TIME
};


namespace gl4
{
	class Shader
	{
	public:


		//initializers
		Shader();
		Shader(const char *vertfilename, const char *fragfilename, const char *geofilename = 0, const char *tesscontrolfilename = 0, const char *tessevalfilename = 0);
		~Shader();

		void init();
		void link();

		bool attachVertexShader(const char *filename);
		bool attachFragmentShader(const char *filename);
		bool attachGeometryShader(const char *filename);
		bool attachTessellationShader(const char *controlFilename, const char *evalFilename);

		GLuint getShaderProgram() { return _shaderProgram; };

		GLint getUniformLocation(int uniform);
		void printUniforms(bool all = false);


	private:
		void _printUniform(const char *name, int uniform, bool set = false);
		GLuint _compileShader(GLenum shaderType, const char *filename, const char *shaderString);

	protected:
		GLuint _shaderProgram;
		GLint _uniformLocations[20];
		const char* _readShaderFile(const char *filename);
		void _printSource(const char *source);
	};
}

#endif