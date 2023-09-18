#include <iostream>

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
{
	mEmptyPipe.pos /= mTileSheetSize;
	mEmptyPipe.size /= mTileSheetSize;
}

bool
GameView::update(float dt)
{
	mTimeSinceLastInput += dt;
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

	return true;
}

bool
GameView::handleEvent(const Event &)
{
	return false;
}

void
GameView::render(RenderTarget &target)
{
	target.clear(Color::Magenta);

	// background
	const glm::vec2 units[4] = {
		{ 0.f, 0.f },
		{ 0.f, 1.f },
		{ 1.f, 0.f },
		{ 1.f, 1.f },
	};
	const std::uint16_t indices[] = { 0, 1, 2, 1, 3, 2 };

	target.setTexture(&mBackground);
	auto base = target.getPrimIndex(6, 4);
	target.addIndices(base, indices + 0, indices + 6);
	auto vertices = target.getVertexArray(4);
	for (int i = 0; i < 4; i++)
	{
		vertices[i].pos = units[i] * mBackground.getSize();
		vertices[i].uv = units[i];
		vertices[i].color = Color::White;
	}

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

	mContext.fonts->get(FontID::Pericles36).draw(
		target,
		ScorePosition,
		std::to_string(mPlayerScore),
		Color::Black);

	target.draw();
}

void
GameView::drawEmptyPipe(RenderTarget &target, glm::vec2 pos)
{
	const glm::vec2 units[4] = {
		{ 0.f, 0.f },
		{ 0.f, 1.f },
		{ 1.f, 0.f },
		{ 1.f, 1.f },
	};
	const std::uint16_t indices[] = { 0, 1, 2, 1, 3, 2 };
	const glm::vec2 pipeSize(Pipe::PipeWidth, Pipe::PipeHeight);

	auto base = target.getPrimIndex(6, 4);
	target.addIndices(base, indices + 0, indices + 6);
	auto vertices = target.getVertexArray(4);
	for (int i = 0; i < 4; i++)
	{
		vertices[i].pos = units[i] * pipeSize + pos;
		vertices[i].uv = units[i] * mEmptyPipe.size + mEmptyPipe.pos;
		vertices[i].color = Color::White;
	}
}

void
GameView::drawStandardPipe(RenderTarget &target, glm::vec2 pos, const Pipe &pipe)
{
	const glm::vec2 units[4] = {
		{ 0.f, 0.f },
		{ 0.f, 1.f },
		{ 1.f, 0.f },
		{ 1.f, 1.f },
	};
	const std::uint16_t indices[] = { 0, 1, 2, 1, 3, 2 };
	const glm::vec2 pipeSize(Pipe::PipeWidth, Pipe::PipeHeight);

	FloatRect srcRect = pipe.getSourceRect();
	srcRect.pos /= mTileSheetSize;
	srcRect.size /= mTileSheetSize;

	auto base = target.getPrimIndex(6, 4);
	target.addIndices(base, indices + 0, indices + 6);
	auto vertices = target.getVertexArray(4);
	for (int i = 0; i < 4; i++)
	{
		vertices[i].pos = units[i] * pipeSize + pos;
		vertices[i].uv = units[i] * srcRect.size + srcRect.pos;
		vertices[i].color = Color::White;
	}
}

void
GameView::drawFallingPipe(RenderTarget &target, glm::vec2 pos, const FallingPipe &pipe)
{
	const glm::vec2 units[4] = {
		{ 0.f, 0.f },
		{ 0.f, 1.f },
		{ 1.f, 0.f },
		{ 1.f, 1.f },
	};
	const std::uint16_t indices[] = { 0, 1, 2, 1, 3, 2 };
	const glm::vec2 pipeSize(Pipe::PipeWidth, Pipe::PipeHeight);

	pos.y -= pipe.getVerticalOffset();

	FloatRect srcRect = pipe.getSourceRect();
	srcRect.pos /= mTileSheetSize;
	srcRect.size /= mTileSheetSize;

	auto base = target.getPrimIndex(6, 4);
	target.addIndices(base, indices + 0, indices + 6);
	auto vertices = target.getVertexArray(4);
	for (int i = 0; i < 4; i++)
	{
		vertices[i].pos = units[i] * pipeSize + pos;
		vertices[i].uv = units[i] * srcRect.size + srcRect.pos;
		vertices[i].color = Color::White;
	}
}

void
GameView::drawFadingPipe(RenderTarget &target, glm::vec2 pos, const FadingPipe &pipe)
{
	const glm::vec2 units[4] = {
		{ 0.f, 0.f },
		{ 0.f, 1.f },
		{ 1.f, 0.f },
		{ 1.f, 1.f },
	};
	const std::uint16_t indices[] = { 0, 1, 2, 1, 3, 2 };
	const glm::vec2 pipeSize(Pipe::PipeWidth, Pipe::PipeHeight);


	FloatRect srcRect = pipe.getSourceRect();
	srcRect.pos /= mTileSheetSize;
	srcRect.size /= mTileSheetSize;

	Color color(255, 255, 255, 255.f * pipe.getAlphaLevel());
	auto base = target.getPrimIndex(6, 4);
	target.addIndices(base, indices + 0, indices + 6);
	auto vertices = target.getVertexArray(4);
	for (int i = 0; i < 4; i++)
	{
		vertices[i].pos = units[i] * pipeSize + pos;
		vertices[i].uv = units[i] * srcRect.size + srcRect.pos;
		vertices[i].color = color;
	}
}

void
GameView::drawRotatingPipe(RenderTarget &target, glm::vec2 pos, const RotatingPipe &pipe)
{
	const glm::vec2 units[4] = {
		{ 0.f, 0.f },
		{ 0.f, 1.f },
		{ 1.f, 0.f },
		{ 1.f, 1.f },
	};
	const std::uint16_t indices[] = { 0, 1, 2, 1, 3, 2 };
	const glm::vec2 pipeSize(Pipe::PipeWidth, Pipe::PipeHeight);

	auto mat4 = glm::translate(
		glm::rotate(
			glm::translate(
				glm::mat4(1.f),
				glm::vec3(pipeSize * .5f, 0.f)),
			pipe.getRotation(),
			glm::vec3(0.f, 0.f, -1.f)),
		glm::vec3(pipeSize * -.5f, 0.f));

	FloatRect srcRect = pipe.getSourceRect();
	srcRect.pos /= mTileSheetSize;
	srcRect.size /= mTileSheetSize;

	auto base = target.getPrimIndex(6, 4);
	target.addIndices(base, indices + 0, indices + 6);
	auto vertices = target.getVertexArray(4);
	for (int i = 0; i < 4; i++)
	{
		vertices[i].pos = glm::vec2(mat4 * glm::vec4(units[i] * pipeSize, 0.f, 1.f)) + pos;
		vertices[i].uv = units[i] * srcRect.size + srcRect.pos;
		vertices[i].color = Color::White;
	}
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

	mPlayerScore += determineScore(waterChain.size());
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
