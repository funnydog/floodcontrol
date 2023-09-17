#pragma once

#include <string>

#include <glm/glm.hpp>

struct GLFWwindow;

class Window
{
public:
	Window();
	~Window();

	Window(const Window &) = delete;
	Window(Window &&) = delete;
	Window& operator=(const Window &) = delete;
	Window& operator=(Window &&) = delete;

	void open(const std::string &title, unsigned width, unsigned height);
	void close();
	bool isClosed() const;
	void display();

	void setTitle(const std::string &title);

	glm::ivec2 getSize() const;
	bool isKeyPressed(int key) const;
	void getMouseState(double &x, double &y, unsigned &buttons);

	static void setContext(Window *window);

	GLFWwindow *getGLFWwindow() const;

private:
	GLFWwindow *mWindow;
	glm::ivec2  mSize;
};
