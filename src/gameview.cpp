#include <iostream>

#include "gameview.hpp"
#include "rendertarget.hpp"
#include "resourceholder.hpp"
#include "texture.hpp"
#include "window.hpp"

namespace
{

static const glm::vec2 BoardOrigin(70.f, 89.f);
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

	mContext.window->setTitle("FloodControl - Score: " + std::to_string(mPlayerScore));

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
	for (int x = 0; x < mBoard.BoardWidth; x++)
	{
		for (int y = 0; y < mBoard.BoardHeight; y++)
		{
			glm::vec2 pos(x, y);

			// underlying empty background
			base = target.getPrimIndex(6, 4);
			target.addIndices(base, indices + 0, indices + 6);
			vertices = target.getVertexArray(4);
			for (int i = 0; i < 4; i++)
			{
				vertices[i].pos = (units[i] + pos) * pipeSize + BoardOrigin;
				vertices[i].uv = units[i] * mEmptyPipe.size + mEmptyPipe.pos;
				vertices[i].color = Color::White;
			}

			// actual pipe
			FloatRect srcRect = mBoard.getSourceRect(x, y);
			srcRect.pos /= mTileSheetSize;
			srcRect.size /= mTileSheetSize;

			base = target.getPrimIndex(6, 4);
			target.addIndices(base, indices + 0, indices + 6);
			vertices = target.getVertexArray(4);
			for (int i = 0; i < 4; i++)
			{
				vertices[i].pos = (units[i] + pos) * pipeSize + BoardOrigin;
				vertices[i].uv = units[i] * srcRect.size + srcRect.pos;
				vertices[i].color = Color::White;
			}
		}
	}

	target.draw();
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
			mBoard.rotatePipe(x, y, false);
			mTimeSinceLastInput = 0.f;
		}
		if (mb & 2)
		{
			mBoard.rotatePipe(x, y, true);
			mTimeSinceLastInput = 0.f;
		}
	}
}
