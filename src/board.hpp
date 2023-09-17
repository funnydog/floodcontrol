#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "pipe.hpp"

class Board
{
public:
	static const int BoardWidth = 8;
	static const int BoardHeight = 10;

public:
	Board();

	void clear();

	void rotatePipe(int x, int y, bool clockwise);
	FloatRect getSourceRect(int x, int y) const;
	Pipe::Type getPipeType(int x, int y) const;
	bool hasConnector(int x, int y, Pipe::Direction dir) const;

	void setType(int x, int y, Pipe::Type type);
	void randomizePipe(int x, int y);

	void fillFromAbove(int x, int y);
	void makeNewPipes(bool dropPipes);
	void resetWater();
	void fillPipe(int x, int y);

	void propagateWater(int x, int y, Pipe::Direction from);
	const std::vector<glm::ivec2>& getWaterChain(int y);

private:
	Pipe& getPipe(int x, int y);
	const Pipe& getPipe(int x, int y) const;

private:
	Pipe mPipes[BoardWidth * BoardHeight];
	std::vector<glm::ivec2> mWaterTracker;
};
