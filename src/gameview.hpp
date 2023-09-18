#pragma once

#include <vector>
#include <memory>

#include "view.hpp"
#include "viewstack.hpp"
#include "board.hpp"
#include "scorezoom.hpp"

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

	void updateScoreZooms(float dt);

	void drawEmptyPipe(RenderTarget &target, glm::vec2 pos);
	void drawStandardPipe(RenderTarget &target, glm::vec2 pos, const Pipe &pipe);
	void drawFallingPipe(RenderTarget &target, glm::vec2 pos, const FallingPipe &pipe);
	void drawRotatingPipe(RenderTarget &target, glm::vec2 pos, const RotatingPipe &pipe);
	void drawFadingPipe(RenderTarget &target, glm::vec2 pos, const FadingPipe &pipe);

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

	std::vector<ScoreZoom> mScoreZooms;
};
