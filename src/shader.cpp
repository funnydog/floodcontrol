#include <iostream>

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include "glcheck.hpp"
#include "shader.hpp"
#include "utility.hpp"

void
ShaderUniform::setFloat(GLfloat value) const noexcept
{
	glCheck(glUniform1f(mLocation, value));
}

void
ShaderUniform::setFloat1fv(const GLfloat *floats, size_t size) const noexcept
{
	glCheck(glUniform1fv(mLocation, size, floats));
}

void
ShaderUniform::setInteger(GLint value) const noexcept
{
	glCheck(glUniform1i(mLocation, value));
}

void
ShaderUniform::setInteger1iv(const GLint *ints, size_t size) const noexcept
{
	glCheck(glUniform1iv(mLocation, size, ints));
}

void
ShaderUniform::setVector2f(GLfloat x, GLfloat y) const noexcept
{
	glCheck(glUniform2f(mLocation, x, y));
}

void
ShaderUniform::setVector2f(const glm::vec2 &value) const noexcept
{
	glCheck(glUniform2fv(mLocation, 1, glm::value_ptr(value)));
}

void
ShaderUniform::setVector2fv(const GLfloat floats[][2], size_t size) const noexcept
{
	glCheck(glUniform2fv(mLocation, size, floats[0]));
}

void
ShaderUniform::setVector3f(GLfloat x, GLfloat y, GLfloat z) const noexcept
{
	glCheck(glUniform3f(mLocation, x, y, z));
}

void
ShaderUniform::setVector3f(const glm::vec3 &value) const noexcept
{
	glCheck(glUniform3fv(mLocation, 1, glm::value_ptr(value)));
}

void
ShaderUniform::setVector3fv(const GLfloat floats[][3], size_t size) const noexcept
{
	glCheck(glUniform3fv(mLocation, size, floats[0]));
}

void
ShaderUniform::setVector4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w) const noexcept
{
	glCheck(glUniform4f(mLocation, x, y, z, w));
}

void
ShaderUniform::setVector4f(const glm::vec4 &value) const noexcept
{
	glCheck(glUniform4fv(mLocation, 1, glm::value_ptr(value)));
}

void
ShaderUniform::setVector4fv(const GLfloat floats[][4], size_t size) const noexcept
{
	glCheck(glUniform4fv(mLocation, size, floats[0]));
}

void
ShaderUniform::setMatrix4(const glm::mat4 &value) const noexcept
{
	glCheck(glUniformMatrix4fv(mLocation, 1, GL_FALSE, glm::value_ptr(value)));
}

void
Shader::create()
{
	if (!mProgram)
	{
		mProgram = glCreateProgram();
	}
}

void
Shader::destroy()
{
	if (mProgram)
	{
		glCheck(glDeleteProgram(mProgram));
	}
}

bool
Shader::attachString(Shader::Type type, const std::string &source) const noexcept
{
	if (!mProgram)
	{
		std::cerr << "Shader::attachString() - undefined program\n";
		return false;
	}

	GLenum glType;
	switch (type)
	{
	case Type::Vertex: glType = GL_VERTEX_SHADER; break;
	case Type::Fragment: glType = GL_FRAGMENT_SHADER; break;
	case Type::Geometry: glType = GL_GEOMETRY_SHADER; break;
	case Type::Compute: glType = GL_COMPUTE_SHADER; break;
	default:
		std::cerr << "Shader::attachString() - undefined shader type"
		          << static_cast<int>(type) << "\n";
		return false;
	}

	GLuint shader = glCreateShader(glType);
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
		std::cerr << "Shader::attachSTring() - compilation failed\n"
		          << message << "\n";
		glCheck(glDeleteShader(shader));
		return false;
	}

	glCheck(glAttachShader(mProgram, shader));
	glCheck(glDeleteShader(shader));
	return true;
}

bool
Shader::attachFile(Type type, const std::filesystem::path &filename) const noexcept
{
	return attachString(type, Utility::loadFile(filename));
}

bool
Shader::link() const noexcept
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
		std::cerr << "Shader::link() - link failed\n"
		          << message << "\n";
		return false;
	}
	return true;
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
Shader::use() const
{
	glCheck(glUseProgram(mProgram));
}
