#include <cassert>

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include "glcheck.hpp"
#include "shader.hpp"
#include "utility.hpp"

ShaderUniform::ShaderUniform(int location)
	: mLocation(location)
{
	assert(location >= 0);
}

void
ShaderUniform::set(const glm::mat4 &matrix)
{
	glCheck(glUniformMatrix4fv(
			mLocation,
			1,
			GL_FALSE,
			glm::value_ptr(matrix)));
}

void
ShaderUniform::set(int value)
{
	glCheck(glUniform1i(mLocation, value));
}

Shader::Shader()
	: mProgram(0)
{
}

Shader::~Shader()
{
	if (mProgram)
	{
		glDeleteProgram(mProgram);
	}
}

void
Shader::attach(const std::string &source, ShaderType shaderType)
{
	if (!mProgram)
	{
		mProgram = glCreateProgram();
	}

	GLenum glType;
	switch (shaderType)
	{
	case ShaderType::Vertex: glType = GL_VERTEX_SHADER; break;
	case ShaderType::Fragment: glType = GL_FRAGMENT_SHADER; break;
	case ShaderType::Geometry: glType = GL_GEOMETRY_SHADER; break;
	case ShaderType::Compute: glType = GL_COMPUTE_SHADER; break;
	default:
		throw std::runtime_error("Unknown shader type");
	}

	unsigned shader = glCreateShader(glType);
	const char *src = source.c_str();
	glCheck(glShaderSource(shader, 1, &src, nullptr));
	glCheck(glCompileShader(shader));

	GLint success;
	glCheck(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
	if (!success)
	{
		GLint length;
		glCheck(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length));
		std::string message(length, 0);
		glCheck(glGetShaderInfoLog(shader, length, nullptr, message.data()));
		glCheck(glDeleteShader(shader));
		throw std::runtime_error(message);
	}

	glCheck(glAttachShader(mProgram, shader));
	glCheck(glDeleteShader(shader));
}

void
Shader::attachFile(const std::filesystem::path &filename, ShaderType shaderType)
{
	attach(Utility::loadFile(filename), shaderType);
}

void
Shader::link()
{
	glCheck(glLinkProgram(mProgram));

	GLint success;
	glCheck(glGetProgramiv(mProgram, GL_LINK_STATUS, &success));
	if (!success)
	{
		GLint length;
		glCheck(glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &length));
		std::string message(length, 0);
		glCheck(glGetProgramInfoLog(mProgram, length, nullptr, message.data()));
		throw std::runtime_error(message);
	}
}

ShaderUniform
Shader::getUniform(const std::string &name) const
{
	auto location = glGetUniformLocation(mProgram, name.c_str());
	if (location == -1)
	{
		throw std::runtime_error(name + " uniform not found");
	}
	return ShaderUniform(location);
}

void
Shader::bind(const Shader *shader)
{
	glUseProgram(shader ? shader->mProgram : 0);
}
