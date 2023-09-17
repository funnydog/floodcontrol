#include <iostream>

#include <GL/glew.h>

#include "glcheck.hpp"

void
glCheckError(const std::filesystem::path &file,
	     unsigned line,
	     std::string_view expression)
{
	const char *error = "unknown error";
	const char *description = "unknown error description";

	switch (glGetError())
	{
	case GL_NO_ERROR:
		return;
	case GL_INVALID_ENUM:
		error = "GL_INVALID_ENUM";
		description = "An unacceptable value has been specified for an enumerated argument.";
		break;
	case GL_INVALID_VALUE:
		error = "GL_INVALID_VALUE";
		description = "A numeric argument is out of range.";
		break;
	case GL_INVALID_OPERATION:
		error = "GL_INVALID_OPERATION";
		description = "The specified operation is not allowed in the current state.";
		break;
	case GL_STACK_OVERFLOW:
		error = "GL_STACK_OVERFLOW";
		description = "This command would cause a stack overflow.";
		break;
	case GL_STACK_UNDERFLOW:
		error = "GL_STACK_UNDERFLOW";
		description = "This command would cause a stack underflow.";
		break;
	case GL_OUT_OF_MEMORY:
		error = "GL_OUT_OF_MEMORY";
		description = "There is not enough memory left to execute the command.";
		break;
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		error = "GL_INVALID_FRAMEBUFFER_OPERATION";
		description = "The object bound to FRAMEBUFFER_BINDING is not complete.";
		break;
	}

	std::cerr << "An internal OpenGL call failed in " << file.filename()
		  << "(" << line << ")."
		  << "\nExpression:\n	" << expression
		  << "\nError description:\n   " << error
		  << "\n   " << description << '\n'
		  << std::endl;
}
