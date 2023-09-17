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

	const glm::vec2 units[4] = {
		{ 0.f, 0.f },
		{ 0.f, 1.f },
		{ 1.f, 0.f },
		{ 1.f, 1.f },
	};
	const std::uint16_t indices[] = { 0, 1, 2, 1, 3, 2 };

	target.setTexture(&mTexture);
	auto base = target.getPrimIndex(6, 4);
	target.addIndices(base, indices + 0, indices + 6);
	auto vertices = target.getVertexArray(4);
	for (int i = 0; i < 4; i++)
	{
		vertices[i].pos = units[i] * mTextureSize;
		vertices[i].uv = units[i];
		vertices[i].color = Color::White;
	}
	target.draw();
}
