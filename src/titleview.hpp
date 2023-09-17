#pragma once

#include <glm/glm.hpp>

#include "view.hpp"
#include "viewstack.hpp"

class TitleView: public View
{
public:
	TitleView(ViewStack &stack, const Context &context);
	virtual ~TitleView() override = default;

	virtual bool update(float dt) override;
	virtual bool handleEvent(const Event &event) override;
	virtual void render(RenderTarget &target) override;

private:
	ViewStack &mViewStack;
	Texture &mTexture;
	glm::vec2 mTextureSize;
};
