#pragma once

#include "rect.hpp"

class Pipe
{
public:
	static constexpr int PipeWidth = 40;
	static constexpr int PipeHeight = 40;
	static constexpr glm::vec2 Size = glm::vec2(PipeWidth, PipeHeight);

	enum Direction
	{
		Top    = 1 << 0,
		Left   = 1 << 1,
		Bottom = 1 << 2,
		Right  = 1 << 3,
	};

	enum Type
	{
		LeftRight,
		TopBottom,
		LeftTop,
		TopRight,
		RightBottom,
		BottomLeft,
		Empty,
	};
public:
	explicit Pipe(Type type = Empty, bool filled = false);

	Type getType() const;
	void setType(Type type);

	bool isFilled() const;
	void setFilled(bool filled);

	void rotate(bool clockwise);

	bool hasConnector(Direction dir) const;
	FloatRect getSourceRect() const;

private:
	Type mType;
	bool mFilled;
};
