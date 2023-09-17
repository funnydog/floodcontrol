#include "pipe.hpp"

namespace
{
static unsigned typeDirections[] = {
	Pipe::Left | Pipe::Right,
	Pipe::Top | Pipe::Bottom,
	Pipe::Left | Pipe::Top,
	Pipe::Top | Pipe::Right,
	Pipe::Right | Pipe::Bottom,
	Pipe::Bottom | Pipe::Left,
	0,
};

static const int textureOffsetX = 1;
static const int textureOffsetY = 1;
static const int texturePaddingX = 1;
static const int texturePaddingY = 1;
}

Pipe::Pipe()
	: mType(Empty)
	, mFilled(false)
{
}

Pipe::Type
Pipe::getType() const
{
	return mType;
}

void
Pipe::setType(Type type)
{
	mType = type;
}

bool
Pipe::isFilled() const
{
	return mFilled;
}

void
Pipe::setFilled(bool filled)
{
	mFilled = filled;
}

void
Pipe::rotate(bool clockwise)
{
	switch (mType)
	{
	case LeftRight:
		mType = TopBottom;
		break;
	case TopBottom:
		mType = LeftRight;
		break;
	case LeftTop:
		mType = clockwise ? TopRight : BottomLeft;
		break;
	case TopRight:
		mType = clockwise ? RightBottom : LeftTop;
		break;
	case RightBottom:
		mType = clockwise ? BottomLeft : TopRight;
		break;
	case BottomLeft:
		mType = clockwise ? LeftTop : RightBottom;
		break;
	case Empty:
		break;
	}
}

bool
Pipe::hasConnector(Direction dir) const
{
	return (typeDirections[mType] & dir) != 0;
}

FloatRect
Pipe::getSourceRect() const
{
	int x = textureOffsetX;
	int y = textureOffsetY;

	if (mFilled)
	{
		x += PipeWidth + texturePaddingX;
	}

	y += mType * (PipeHeight + texturePaddingY);

	return {{x, y}, {PipeWidth, PipeHeight}};
}
