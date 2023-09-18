#pragma once

#include "view.hpp"
#include "viewstack.hpp"

class GameOverView: public View
{
public:
	GameOverView(ViewStack &stack, const Context &context);
	virtual ~GameOverView() override = default;

	virtual bool update(float dt) override;
	virtual bool handleEvent(const Event &event) override;
	virtual void render(RenderTarget &target) override;

private:
	ViewStack &mStack;
	Font &mFont;
	float mTimer;
};
