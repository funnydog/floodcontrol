#pragma once

#include <vector>
#include <map>
#include <memory>

#include <glm/glm.hpp>

#include "pipe.hpp"
#include "rotatingpipe.hpp"
#include "fallingpipe.hpp"
#include "fadingpipe.hpp"

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
	bool hasConnector(int x, int y, Pipe::Direction dir) const;

	Pipe::Type getType(int x, int y) const;
	void setType(int x, int y, Pipe::Type type);
	void randomizePipe(int x, int y);

	void fillFromAbove(int x, int y);
	void makeNewPipes(bool dropPipes);
	void resetWater();
	void fillPipe(int x, int y);

	void propagateWater(int x, int y, Pipe::Direction from);
	const std::vector<glm::ivec2>& getWaterChain(int y);

	bool arePipesAnimating() const;
	void updateAnimatedPipes();

	void addFallingPipe(int x, int y, Pipe::Type type, int verticalOffset);
	void addRotatingPipe(int x, int y, Pipe::Type type, bool clockwise);
	void addFadingPipe(int x, int y, Pipe::Type type);

	const Pipe& getPipe(int x, int y) const;
	Pipe& getPipe(int x, int y);

private:
	void updateFallingPipes();
	void updateRotatingPipes();
	void updateFadingPipes();

private:
	Pipe mPipes[BoardWidth * BoardHeight];
	std::vector<glm::ivec2> mWaterTracker;

public:
	std::map<std::pair<int, int>, std::unique_ptr<FallingPipe>> mFallingPipes;
	std::map<std::pair<int, int>, std::unique_ptr<RotatingPipe>> mRotatingPipes;
	std::map<std::pair<int, int>, std::unique_ptr<FadingPipe>> mFadingPipes;
};
