#include <iostream>

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "gameview.hpp"

#include "font.hpp"
#include "rendertarget.hpp"
#include "resourceholder.hpp"
#include "texture.hpp"
#include "window.hpp"

namespace
{

static const glm::vec2 BoardOrigin(70.f, 89.f);
static const glm::vec2 ScorePosition(605.f, 215.f);
static const float MinTimeSinceLastInput = 0.25f;

static const glm::vec2 WaterPosition(478.f, 338.f);
static const glm::vec2 WaterSize(244.f, 297.f);
static const glm::vec2 WaterOverlayStart(85.f, 245.f);

static const float MaxFloodCounter = 100.f;
static const float TimeBetweenFloodIncreases = 1.f;

static const float FloodAccelerationPerLevel = 0.5f;
static const glm::vec2 LevelPosition(512.f, 215.f);

}

GameView::GameView(ViewStack &stack, const Context &context)
	: mViewStack(stack)
	, mContext(context)
	, mBackground(context.textures->get(TextureID::Background))
	, mTileSheet(context.textures->get(TextureID::TileSheet))
	, mTileSheetSize(mTileSheet.getSize())
	, mEmptyPipe({1.f, 247.f}, {40.f, 40.f})
	, mBoard()
	, mPlayerScore(0)
	, mTimeSinceLastInput(0.f)
	, mTimeSinceLastIncrease(0.f)
	, mFloodCount(0.f)
	, mFloodIncreaseAmount(0.5f)
	, mCurrentLevel(0)
	, mLinesCompleted(0)
{
}

bool
GameView::update(float dt)
{
	mTimeSinceLastInput += dt;
	mTimeSinceLastIncrease += dt;
	if (mTimeSinceLastIncrease >= TimeBetweenFloodIncreases)
	{
		mTimeSinceLastIncrease -= TimeBetweenFloodIncreases;
		mFloodCount += mFloodIncreaseAmount;
		if (mFloodCount > MaxFloodCounter)
		{
			mViewStack.pushView(ViewID::GameOver);
		}
	}

	if (mBoard.arePipesAnimating())
	{
		mBoard.updateAnimatedPipes();
	}
	else
	{
		if (mTimeSinceLastInput >= MinTimeSinceLastInput)
		{
			double mx, my;
			unsigned mb;
			mContext.window->getMouseState(mx, my, mb);
			handleMouseInput(mx, my, mb);
		}

		mBoard.resetWater();
		for (int y = 0; y < Board::BoardHeight; y++)
		{
			checkScoringChain(mBoard.getWaterChain(y));
		}
		mBoard.makeNewPipes(true);
	}

	updateScoreZooms(dt);

	return true;
}

void
GameView::updateScoreZooms(float dt)
{
	for (auto it = mScoreZooms.begin();
	     it != mScoreZooms.end();)
	{
		it->update(dt);
		if (it->isCompleted())
		{
			it = mScoreZooms.erase(it);
		}
		else
		{
			++it;
		}
	}
}

bool
GameView::handleEvent(const Event &event)
{
	if (const auto ep(std::get_if<KeyPressed>(&event)); ep
	    && ep->key == GLFW_KEY_P)
	{
		mViewStack.pushView(ViewID::Paused);
		return true;
	}
	return false;
}

void
GameView::render(RenderTarget &target)
{
	target.clear(Color::Magenta);

	// background
	target.blit(mBackground, nullptr, {0.f, 0.f}, Color::White);

	const glm::vec2 pipeSize(Pipe::PipeWidth, Pipe::PipeHeight);
	target.setTexture(&mTileSheet);
	auto pair = std::make_pair(0, 0);
	for (pair.first = 0; pair.first < mBoard.BoardWidth; pair.first++)
	{
		for (pair.second = 0; pair.second < mBoard.BoardHeight; pair.second++)
		{
			auto pos = glm::vec2(pair.first, pair.second) * pipeSize + BoardOrigin;

			drawEmptyPipe(target, pos);

			if (auto it = mBoard.mRotatingPipes.find(pair); it != mBoard.mRotatingPipes.end())
			{
				drawRotatingPipe(target, pos, *it->second);
			}
			else if (auto it = mBoard.mFadingPipes.find(pair); it != mBoard.mFadingPipes.end())
			{
				drawFadingPipe(target, pos, *it->second);
			}
			else if (auto it = mBoard.mFallingPipes.find(pair); it != mBoard.mFallingPipes.end())
			{
				drawFallingPipe(target, pos, *it->second);
			}
			else
			{
				drawStandardPipe(target, pos, mBoard.getPipe(pair.first, pair.second));
			}
		}
	}

	// level
	auto &font = mContext.fonts->get(FontID::Pericles36);
	font.draw(
		target,
		LevelPosition,
		std::to_string(mCurrentLevel),
		Color::Black);

	// points
	font.draw(
		target,
		ScorePosition,
		std::to_string(mPlayerScore),
		Color::Black);

	// scorezoom
        auto winCenter = glm::vec3(mContext.window->getSize(), 0.f) * 0.5f;
	for (auto &scoreZoom: mScoreZooms)
	{
		auto textSize = font.getSize(scoreZoom.text);
		auto scale = scoreZoom.getScale();
		glm::mat4 mat4 = glm::translate(
			glm::scale(
				glm::translate(
					glm::mat4(1.f),
					winCenter),
				glm::vec3(scale, scale, 1.f)),
			glm::vec3(textSize * -0.5f, 0.f));
		font.draw(target, mat4, scoreZoom.text, scoreZoom.drawColor);
	}

	// flood
	float waterHeight = 244.f * mFloodCount / 100;
	FloatRect srcRect = {
		{ 85.f, 245.f + 244.f - waterHeight },
		{ 297.f, waterHeight },
	};

	FloatRect dstRect = {
		{ 478.f, 338.f + 244.f - waterHeight },
		srcRect.size
	};

	target.blit(mBackground,
		    &srcRect,
		    dstRect.pos,
		    Color(255, 255, 255, 180));

	target.draw();
}

void
GameView::drawEmptyPipe(RenderTarget &target, glm::vec2 pos)
{
	target.blit(mTileSheet, &mEmptyPipe, pos, Color::White);
}

void
GameView::drawStandardPipe(RenderTarget &target, glm::vec2 pos, const Pipe &pipe)
{
	auto srcRect = pipe.getSourceRect();
	target.blit(mTileSheet, &srcRect, pos, Color::White);
}

void
GameView::drawFallingPipe(RenderTarget &target, glm::vec2 pos, const FallingPipe &pipe)
{
	auto srcRect = pipe.getSourceRect();
	target.blit(
		mTileSheet,
		&srcRect,
		{pos.x, pos.y - pipe.getVerticalOffset()},
		Color::White);
}

void
GameView::drawFadingPipe(RenderTarget &target, glm::vec2 pos, const FadingPipe &pipe)
{
	auto srcRect = pipe.getSourceRect();
	target.blit(
		mTileSheet,
		&srcRect,
		pos,
		Color(255, 255, 255, 255.f * pipe.getAlphaLevel()));
}

void
GameView::drawRotatingPipe(RenderTarget &target, glm::vec2 pos, const RotatingPipe &pipe)
{
	auto srcRect = pipe.getSourceRect();
	target.blit(
		mTileSheet,
		&srcRect,
		pos,
		Color::White,
		pipe.getRotation(),
		glm::vec2(Pipe::PipeWidth, Pipe::PipeHeight) * 0.5f,
		1.f);
}

int
GameView::determineScore(int squareCount)
{
	return static_cast<int>((std::pow(squareCount / 5.f, 2.f) + squareCount) * 10.f);
}

void
GameView::checkScoringChain(const std::vector<glm::ivec2> &waterChain)
{
	if (waterChain.empty())
	{
		return;
	}

	auto lastPipe = waterChain.back();
	if (lastPipe.x != Board::BoardWidth - 1
	    || !mBoard.hasConnector(lastPipe.x, lastPipe.y, Pipe::Right))
	{
		return;
	}

	auto score = determineScore(waterChain.size());
	mScoreZooms.emplace_back(
		std::to_string(score),
		Color(255, 0, 0, 102));

	mPlayerScore += score;
	mFloodCount -= score / 10.f;
	if (mFloodCount < 0.f)
	{
		mFloodCount = 0.f;
	}

	mLinesCompleted++;
	if (mLinesCompleted >= 10)
	{
		startNewLevel();
	}

	for (auto pos: waterChain)
	{
		mBoard.addFadingPipe(pos.x, pos.y, mBoard.getType(pos.x, pos.y));
		mBoard.setType(pos.x, pos.y, Pipe::Empty);
	}
}

void
GameView::handleMouseInput(double mx, double my, unsigned mb)
{
	int x = static_cast<int>((mx - BoardOrigin.x) / Pipe::PipeWidth);
	int y = static_cast<int>((my - BoardOrigin.y) / Pipe::PipeHeight);

	if  (0 <= x && x < Board::BoardWidth && 0 <= y && y < Board::BoardHeight)
	{
		if (mb & 1)
		{
			mBoard.addRotatingPipe(x, y, mBoard.getType(x, y), false);
			mBoard.rotatePipe(x, y, false);
			mTimeSinceLastInput = 0.f;
		}
		if (mb & 2)
		{
			mBoard.addRotatingPipe(x, y, mBoard.getType(x, y), true);
			mBoard.rotatePipe(x, y, true);
			mTimeSinceLastInput = 0.f;
		}
	}
}

void
GameView::startNewLevel()
{
	mCurrentLevel++;
	mLinesCompleted = 0;
	mFloodCount = 0.f;
	mFloodIncreaseAmount += FloodAccelerationPerLevel;
	mBoard.clear();
	mBoard.makeNewPipes(false);
}
