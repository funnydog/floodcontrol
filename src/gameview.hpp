#pragma once

#include "view.hpp"
#include "viewstack.hpp"
#include "board.hpp"

class GameView: public View
{
public:
	GameView(ViewStack &stack, const Context &context);
	virtual ~GameView() override = default;

	virtual bool update(float dt) override;
	virtual bool handleEvent(const Event &event) override;
	virtual void render(RenderTarget &target) override;

private:
	static int determineScore(int squareCount);
	void checkScoringChain(const std::vector<glm::ivec2> &waterChain);

	void handleMouseInput(double mx, double my, unsigned mb);

private:
	ViewStack &mViewStack;
	const Context &mContext;
	Texture &mBackground;
	Texture &mTileSheet;
	glm::vec2 mTileSheetSize;
	FloatRect mEmptyPipe;

	Board mBoard;
	int mPlayerScore;
	float mTimeSinceLastInput;
};
