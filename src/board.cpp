#include <cassert>

#include "board.hpp"
#include "utility.hpp"

namespace
{
}

Board::Board()
	: mPipes()
{
}

void
Board::clear()
{
	for (auto &pipe: mPipes)
	{
		pipe.setType(Pipe::Empty);
		pipe.setFilled(false);
	}
}

const Pipe&
Board::getPipe(int x, int y) const
{
	assert(0 <= y && y < BoardHeight && 0 <= x && x < BoardWidth
	       && "Coordinates out of the board");

	return mPipes[y * BoardWidth + x];
}

Pipe &
Board::getPipe(int x, int y)
{
	return const_cast<Pipe&>(static_cast<const Board *>(this)->getPipe(x, y));
}

void
Board::rotatePipe(int x, int y, bool clockwise)
{
	getPipe(x, y).rotate(clockwise);
}

FloatRect
Board::getSourceRect(int x, int y) const
{
	return getPipe(x, y).getSourceRect();
}

Pipe::Type
Board::getPipeType(int x, int y) const
{
	return getPipe(x, y).getType();
}

bool
Board::hasConnector(int x, int y, Pipe::Direction dir) const
{
	return getPipe(x, y).hasConnector(dir);
}

void
Board::setType(int x, int y, Pipe::Type type)
{
	getPipe(x, y).setType(type);
}

void
Board::randomizePipe(int x, int y)
{
	getPipe(x, y).setType(
		static_cast<Pipe::Type>(Utility::randomInt(Pipe::BottomLeft + 1)));
}

void
Board::fillFromAbove(int x, int y)
{
	assert(0 <= y && y < BoardHeight && 0 <= x && x < BoardWidth
	       && "Coordinates out of the board");

	int rowLookup = y - 1;
	while (rowLookup >= 0)
	{
		auto type = getPipeType(x, rowLookup);
		if (type != Pipe::Empty)
		{
			setType(x, y, type);
			setType(x, rowLookup, Pipe::Empty);
			rowLookup = -1;
		}
		rowLookup--;
	}
}

void
Board::makeNewPipes(bool dropPipes)
{
	if (dropPipes)
	{
		for (int x = 0; x < BoardWidth; x++)
		{
			for (int y = BoardHeight - 1; y >= 0; y--)
			{
				if (getPipeType(x, y) == Pipe::Empty)
				{
					fillFromAbove(x, y);
				}
			}
		}
	}
	for (int y = 0; y < BoardHeight; y++)
	{
		for (int x = 0; x < BoardWidth; x++)
		{
			if (getPipeType(x, y) == Pipe::Empty)
			{
				randomizePipe(x, y);
			}
		}
	}
}

void
Board::resetWater()
{
	for (auto &pipe: mPipes)
	{
		pipe.setFilled(false);
	}
}

void
Board::fillPipe(int x, int y)
{
	getPipe(x, y).setFilled(true);
}

void
Board::propagateWater(int x, int y, Pipe::Direction from)
{
	if (0 <= y && y < BoardHeight && 0 <= x && x < BoardWidth)
	{
		auto &pipe = getPipe(x, y);
		if (pipe.hasConnector(from) && !pipe.isFilled())
		{
			pipe.setFilled(true);
			mWaterTracker.emplace_back(x, y);
			for (int i = 0; i < 4; i++)
			{
				auto dst = static_cast<Pipe::Direction>(1<<i);
				if (dst == from || !pipe.hasConnector(dst))
				{
					continue;
				}
				switch (dst)
				{
				case Pipe::Left:
					propagateWater(x-1, y, Pipe::Right);
					break;
				case Pipe::Right:
					propagateWater(x+1, y, Pipe::Left);
					break;
				case Pipe::Top:
					propagateWater(x, y-1, Pipe::Bottom);
					break;
				case Pipe::Bottom:
					propagateWater(x, y+1, Pipe::Top);
					break;
				}
			}
		}
	}
}

const std::vector<glm::ivec2>&
Board::getWaterChain(int y)
{
	mWaterTracker.clear();
	propagateWater(0, y, Pipe::Left);

	return mWaterTracker;
}
