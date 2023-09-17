#pragma once

#include <unordered_map>

#include "event.hpp"

struct GLFWwindow;

class Window;

class EventQueue
{
public:
	EventQueue();
	~EventQueue();

	void poll();
	bool empty() const;
	bool pop(Event &event);

	void track(Window &window);

private:
	void add(Event &&event);

	static EventQueue& getEventQueue(GLFWwindow *window);

	static void windowPosCallback(GLFWwindow *window, int x, int y);
	static void windowSizeCallback(GLFWwindow *window, int width, int height);
	static void windowCloseCallback(GLFWwindow *window);
	static void windowRefreshCallback(GLFWwindow *window);
	static void windowFocusCallback(GLFWwindow *window, int focused);
	static void windowIconifyCallback(GLFWwindow *window, int iconified);
	static void windowMaximizeCallback(GLFWwindow *window, int maximized);
	static void windowScaleCallback(GLFWwindow *window, float xscale, float yscale);
	static void framebufferSizeCallback(GLFWwindow *window, int width, int height);
	static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
	static void cursorPosCallback(GLFWwindow *window, double x, double y);
	static void cursorEnterCallback(GLFWwindow *window, int entered);
	static void scrollCallback(GLFWwindow *window, double x, double y);
	static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
	static void charCallback(GLFWwindow *window, unsigned codepoint);

private:
	static constexpr std::size_t QUEUE_SIZE = 256;

private:
	std::unordered_map<GLFWwindow *, Window *> mWindows;
	Event mEvents[QUEUE_SIZE];
	unsigned mRead;
	unsigned mWrite;
};
