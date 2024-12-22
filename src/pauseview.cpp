#include <GLFW/glfw3.h>

#include "pauseview.hpp"

#include "font.hpp"
#include "rect.hpp"
#include "rendertarget.hpp"
#include "resourceholder.hpp"
#include "window.hpp"

namespace
{
static const FloatRect Obscured{ {30.f, 89.f}, {400.f, 400.f} };
}

PauseView::PauseView(ViewStack &stack, const Context &context)
	: mStack(stack)
	, mFont(context.fonts->get(FontID::Pericles36))
	, mWindow(*context.window)
{
}

bool
PauseView::update(float)
{
	return true;
}

bool
PauseView::handleEvent(const Event &event)
{
	if (const auto ep(std::get_if<KeyPressed>(&event)); ep)
	{
		if (ep->key == GLFW_KEY_ESCAPE)
		{
			mStack.popView();
		}
		return true;
	}
	return false;
}

void
PauseView::render(RenderTarget &target)
{
	const glm::vec2 units[4] = {
		{ 0.f, 0.f },
		{ 0.f, 1.f },
		{ 1.f, 0.f },
		{ 1.f, 1.f },
	};
	const std::uint16_t indices[] = { 0, 1, 2, 1, 3, 2 };

	target.setTexture(nullptr);
	auto base = target.getPrimIndex(6, 4);
	target.addIndices(base, indices + 0, indices + 6);
	auto vertices = target.getVertexArray(4);
	for (int i = 0; i < 4; i++)
	{
		vertices[i].pos = units[i] * Obscured.size + Obscured.pos;
		vertices[i].uv = units[i];
		vertices[i].color = Color::Black;
	}

	// soft background
	glm::vec2 winSize = mWindow.getSize();
	base = target.getPrimIndex(6, 4);
	target.addIndices(base, indices + 0, indices + 6);
	vertices = target.getVertexArray(4);
	for (int i = 0; i < 4; i++)
	{
		vertices[i].pos = units[i] * winSize;
		vertices[i].uv = units[i];
		vertices[i].color = Color(0, 0, 0, 120);
	}

	// message
	const std::string message = "GAME PAUSED";

	target.draw(message, (winSize - mFont.getSize(message)) * 0.5f, mFont, Color::White);
	target.draw();
}
