#include "gameoverview.hpp"

#include <glm/glm.hpp>

#include "color.hpp"
#include "font.hpp"
#include "rendertarget.hpp"
#include "resourceholder.hpp"

namespace
{
static const glm::vec2 gameOverLocation(200.f, 260.f);
}

GameOverView::GameOverView(ViewStack &stack, const Context &context)
	: mStack(stack)
	, mFont(context.fonts->get(FontID::Pericles36))
	, mTimer(10.f)
{
}

bool
GameOverView::update(float dt)
{
	mTimer -= dt;
	if (mTimer < 0.f)
	{
		mStack.clearStack();
		mStack.pushView(ViewID::Title);
	}
	return true;
}

bool
GameOverView::handleEvent(const Event &event)
{
	if (const auto ep(std::get_if<KeyPressed>(&event)); ep)
	{
		mStack.clearStack();
		mStack.pushView(ViewID::Title);
		return true;
	}
	return false;
}

void
GameOverView::render(RenderTarget &target)
{
	target.draw("G A M E  O V E R !", gameOverLocation, mFont, Color::Yellow);
	target.draw();
}
