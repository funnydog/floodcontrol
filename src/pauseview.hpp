#pragma once

#include "view.hpp"
#include "viewstack.hpp"

class PauseView: public View
{
public:
	PauseView(ViewStack &stack, const Context &context);
	virtual ~PauseView() override = default;

	virtual bool update(float dt) override;
	virtual bool handleEvent(const Event &event) override;
	virtual void render(RenderTarget &target) override;

private:
	ViewStack &mStack;
	Font &mFont;
	Window &mWindow;
};
