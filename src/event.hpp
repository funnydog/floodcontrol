#pragma once

#include <variant>

class Window;

struct WindowMoved
{
	Window *window;
	int x;
	int y;
};

struct WindowResized
{
	Window *window;
	int width;
	int height;
};

struct WindowClosed
{
	Window *window;
};

struct WindowRefreshed
{
	Window *window;
};

struct WindowFocused
{
	Window *window;
};

struct WindowDefocused
{
	Window *window;
};

struct WindowIconified
{
	Window *window;
};

struct WindowUniconified
{
	Window *window;
};

struct WindowMaximized
{
	Window *window;
};

struct WindowUnmaximized
{
	Window *window;
};

struct WindowScaleChanged
{
	Window *window;
	float x;
	float y;
};

struct KeyPressed
{
	Window *window;
	int key;
	int scancode;
	int mod;
};

struct KeyRepeated
{
	Window *window;
	int key;
	int scancode;
	int mod;
};

struct KeyReleased
{
	Window *window;
	int key;
	int scancode;
	int mod;
};

struct MouseButtonPressed
{
	Window *window;
	int button;
	int mods;
};

struct MouseButtonReleased
{
	Window *window;
	int button;
	int mods;
};

struct MouseCursorMoved
{
	Window *window;
	double x;
	double y;
};

struct MouseCursorEntered
{
	Window *window;
};

struct MouseCursorLeft
{
	Window *window;
};

struct MouseScrolled
{
	Window *window;
	double x;
	double y;
};

struct CodepointInput
{
	Window *window;
	unsigned codepoint;
};

struct FramebufferResized
{
	Window *window;
	int width;
	int height;
};

typedef std::variant<
	WindowMoved,
	WindowResized,
	WindowClosed,
	WindowRefreshed,
	WindowFocused,
	WindowDefocused,
	WindowIconified,
	WindowUniconified,
	WindowMaximized,
	WindowUnmaximized,
	WindowScaleChanged,
	KeyPressed,
	KeyRepeated,
	KeyReleased,
	MouseButtonPressed,
	MouseButtonReleased,
	MouseCursorMoved,
	MouseCursorEntered,
	MouseCursorLeft,
	MouseScrolled,
	CodepointInput,
	FramebufferResized> Event;
