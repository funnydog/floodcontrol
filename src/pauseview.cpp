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
	target.draw(Obscured.pos, Obscured.size, Color::Black);

	glm::vec2 winSize = mWindow.getSize();
	target.draw(glm::vec2(0.f), winSize, Color(0, 0, 0, 120));

	// message
	const std::string message = "GAME PAUSED";
	target.draw("GAME PAUSED", (winSize - mFont.getSize(message)) * 0.5f, mFont, Color::White);
}
