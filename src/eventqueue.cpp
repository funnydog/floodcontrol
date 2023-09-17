#include <cassert>
#include <iostream>

#include <GLFW/glfw3.h>

#include "eventqueue.hpp"
#include "window.hpp"

EventQueue::EventQueue()
	: mWindows()
	, mEvents()
	, mRead(0)
	, mWrite(0)
{
}

EventQueue::~EventQueue()
{
	for (auto [wptr, _]: mWindows)
	{
		glfwSetWindowPosCallback(wptr, nullptr);
		glfwSetWindowSizeCallback(wptr, nullptr);
		glfwSetWindowCloseCallback(wptr, nullptr);
		glfwSetWindowRefreshCallback(wptr, nullptr);
		glfwSetWindowFocusCallback(wptr, nullptr);
		glfwSetWindowIconifyCallback(wptr, nullptr);
		glfwSetWindowMaximizeCallback(wptr, nullptr);
		glfwSetWindowContentScaleCallback(wptr, nullptr);
		glfwSetFramebufferSizeCallback(wptr, nullptr);
		glfwSetMouseButtonCallback(wptr, nullptr);
		glfwSetCursorPosCallback(wptr, nullptr);
		glfwSetCursorEnterCallback(wptr, nullptr);
		glfwSetScrollCallback(wptr, nullptr);
		glfwSetKeyCallback(wptr, nullptr);
		glfwSetCharCallback(wptr, nullptr);
	}
}

void
EventQueue::poll()
{
	glfwPollEvents();
}

bool
EventQueue::empty() const
{
	return mRead == mWrite;
}

bool
EventQueue::pop(Event &event)
{
	if (mRead == mWrite)
	{
		return false;
	}

	event = mEvents[mRead & (QUEUE_SIZE - 1)];
	++mRead;
	return true;
}

void
EventQueue::track(Window &window)
{
	auto wptr = window.getGLFWwindow();
	if (!wptr)
	{
		return;
	}
	auto [_, inserted] = mWindows.insert(std::make_pair(wptr, &window));
	assert(inserted && "Window already registered.");
	glfwSetWindowUserPointer(wptr, this);
	glfwSetWindowPosCallback(wptr, windowPosCallback);
	glfwSetWindowSizeCallback(wptr, windowSizeCallback);
	glfwSetWindowCloseCallback(wptr, windowCloseCallback);
	glfwSetWindowRefreshCallback(wptr, windowRefreshCallback);
	glfwSetWindowFocusCallback(wptr, windowFocusCallback);
	glfwSetWindowIconifyCallback(wptr, windowIconifyCallback);
	glfwSetWindowMaximizeCallback(wptr, windowMaximizeCallback);
	glfwSetWindowContentScaleCallback(wptr, windowScaleCallback);
	glfwSetFramebufferSizeCallback(wptr, framebufferSizeCallback);
	glfwSetMouseButtonCallback(wptr, mouseButtonCallback);
	glfwSetCursorPosCallback(wptr, cursorPosCallback);
	glfwSetCursorEnterCallback(wptr, cursorEnterCallback);
	glfwSetScrollCallback(wptr, scrollCallback);
	glfwSetKeyCallback(wptr, keyCallback);
	glfwSetCharCallback(wptr, charCallback);
}

void
EventQueue::add(Event &&event)
{
	if (mRead + QUEUE_SIZE == mWrite)
	{
		std::cerr << "EventQueue::add() failed - queue full, event lost\n";
		return;
	}

	mEvents[mWrite & (QUEUE_SIZE - 1)] = std::move(event);
	++mWrite;
}

EventQueue&
EventQueue::getEventQueue(GLFWwindow *window)
{
	return *reinterpret_cast<EventQueue*>(glfwGetWindowUserPointer(window));
}

void
EventQueue::windowPosCallback(GLFWwindow *window, int x, int y)
{
	auto &ev = getEventQueue(window);
	ev.add(WindowMoved{ev.mWindows[window], x, y});
}

void
EventQueue::windowSizeCallback(GLFWwindow *window, int width, int height)
{
	auto &ev = getEventQueue(window);
	ev.add(WindowResized{ev.mWindows[window], width, height});
}

void
EventQueue::windowCloseCallback(GLFWwindow *window)
{
	auto &ev = getEventQueue(window);
	ev.add(WindowClosed{ev.mWindows[window]});

	// untrack the window
	ev.mWindows.erase(window);
}

void
EventQueue::windowRefreshCallback(GLFWwindow *window)
{
	auto &ev = getEventQueue(window);
	ev.add(WindowRefreshed{ev.mWindows[window]});
}

void
EventQueue::windowFocusCallback(GLFWwindow *window, int focused)
{
	auto &ev = getEventQueue(window);
	if (focused)
	{
		ev.add(WindowFocused{ev.mWindows[window]});
	}
	else
	{
		ev.add(WindowDefocused{ev.mWindows[window]});
	}
}

void
EventQueue::windowIconifyCallback(GLFWwindow *window, int iconified)
{
	auto &ev = getEventQueue(window);
	if (iconified)
	{
		ev.add(WindowIconified{ev.mWindows[window]});
	}
	else
	{
		ev.add(WindowUniconified{ev.mWindows[window]});
	}
}

void
EventQueue::windowMaximizeCallback(GLFWwindow *window, int maximized)
{
	auto &ev = getEventQueue(window);
	if (maximized)
	{
		ev.add(WindowMaximized{ev.mWindows[window]});
	}
	else
	{
		ev.add(WindowUnmaximized{ev.mWindows[window]});
	}
}

void
EventQueue::windowScaleCallback(GLFWwindow *window, float xscale, float yscale)
{
	auto &ev = getEventQueue(window);
	ev.add(WindowScaleChanged{ev.mWindows[window], xscale, yscale});
}

void
EventQueue::framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
	auto &ev = getEventQueue(window);
	ev.add(FramebufferResized{ev.mWindows[window], width, height});
}

void
EventQueue::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
	auto &ev = getEventQueue(window);
	switch (action)
	{
	case GLFW_PRESS:
		ev.add(MouseButtonPressed{ev.mWindows[window], button, mods});
		break;

	case GLFW_RELEASE:
		ev.add(MouseButtonReleased{ev.mWindows[window], button, mods});
		break;

	default:
		break;
	}
}

void
EventQueue::cursorPosCallback(GLFWwindow *window, double x, double y)
{
	auto &ev = getEventQueue(window);
	ev.add(MouseCursorMoved{ev.mWindows[window], x, y});
}

void
EventQueue::cursorEnterCallback(GLFWwindow *window, int entered)
{
	auto &ev = getEventQueue(window);
	if (entered)
	{
		ev.add(MouseCursorEntered{ev.mWindows[window]});
	}
	else
	{
		ev.add(MouseCursorLeft{ev.mWindows[window]});
	}
}

void
EventQueue::scrollCallback(GLFWwindow *window, double x, double y)
{
	auto &ev = getEventQueue(window);
	ev.add(MouseScrolled{ev.mWindows[window], x, y});
}

void
EventQueue::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	auto &ev = getEventQueue(window);

	switch (action)
	{
	case GLFW_PRESS:
		ev.add(KeyPressed{ev.mWindows[window], key, scancode, mods});
		break;

	case GLFW_RELEASE:
		ev.add(KeyReleased{ev.mWindows[window], key, scancode, mods});
		break;

	case GLFW_REPEAT:
		ev.add(KeyRepeated{ev.mWindows[window], key, scancode, mods});
		break;

	default:
		break;
	}
}

void
EventQueue::charCallback(GLFWwindow *window, unsigned codepoint)
{
	auto &ev = getEventQueue(window);
	ev.add(CodepointInput{ev.mWindows[window], codepoint});

}
