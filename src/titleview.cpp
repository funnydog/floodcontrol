#include <GLFW/glfw3.h>

#include "titleview.hpp"

#include "color.hpp"
#include "resources.hpp"
#include "resourceholder.hpp"
#include "rendertarget.hpp"
#include "texture.hpp"

TitleView::TitleView(ViewStack &stack, const Context &context)
	: mViewStack(stack)
	, mTexture(context.textures->get(TextureID::TitleScreen))
	, mTextureSize(mTexture.getSize())
{
}

bool
TitleView::update(float)
{
	return true;
}

bool
TitleView::handleEvent(const Event &event)
{
	if (const auto ep(std::get_if<KeyPressed>(&event)); ep
	    && ep->key == GLFW_KEY_SPACE)
	{
		mViewStack.popView();
		mViewStack.pushView(ViewID::GamePlay);
		return true;
	}
	return false;
}

void
TitleView::render(RenderTarget &target)
{
	target.clear(Color::Black);
	target.draw(mTexture, glm::vec2(0.f), mTextureSize);
}
