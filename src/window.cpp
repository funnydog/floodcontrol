#include <stdexcept>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "window.hpp"

Window::Window()
	: mWindow(nullptr)
	, mSize{0, 0}
{
}

Window::~Window()
{
	if (mWindow)
	{
		glfwMakeContextCurrent(nullptr);
		glfwDestroyWindow(mWindow);
	}
}

void
Window::open(const std::string &title, unsigned width, unsigned height)
{
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	mWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	if (!mWindow)
	{
		const char *error;
		glfwGetError(&error);
		throw std::runtime_error(error);
	}
	setContext(this);

	glfwSwapInterval(1);
	mSize.x = width;
	mSize.y = height;
}

void
Window::close()
{
	glfwSetWindowShouldClose(mWindow, GLFW_TRUE);
}

bool
Window::isClosed() const
{
	return glfwWindowShouldClose(mWindow);
}

void
Window::display()
{
	glfwSwapBuffers(mWindow);
}

void
Window::setTitle(const std::string &title)
{
	if (mWindow)
	{
		glfwSetWindowTitle(mWindow, title.c_str());
	}
}

glm::ivec2
Window::getSize() const
{
	return mSize;
}

bool
Window::isKeyPressed(int key) const
{
	return glfwGetKey(mWindow, key) == GLFW_PRESS;
}

void
Window::getMouseState(double &x, double &y, unsigned &buttons)
{
	glfwGetCursorPos(mWindow, &x, &y);
	buttons = 0;
	for (int i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++)
	{
		buttons |= (glfwGetMouseButton(mWindow, i) == GLFW_PRESS) << i;
	}
}

void
Window::setContext(Window *window)
{
	if (!window)
	{
		glfwMakeContextCurrent(nullptr);
		return;
	}

	glfwMakeContextCurrent(window->mWindow);
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		glfwTerminate();
		throw std::runtime_error(
			reinterpret_cast<const char *>(glewGetErrorString(err)));
	}
	if (!GLEW_ARB_texture_storage)
	{
		throw std::runtime_error("ARB_texture_storage required!");
	}
}

GLFWwindow*
Window::getGLFWwindow() const
{
	return mWindow;
}
