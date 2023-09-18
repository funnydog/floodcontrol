#include "rotatingpipe.hpp"

namespace
{

static const float RotationRate = 3.141592654f / 2.f / 10;

}

RotatingPipe::RotatingPipe(Type type, bool clockwise)
	: Pipe(type)
	, mRotation(0.f)
	, mClockwise(clockwise)
	, mTicksRemaining(10)
{
}

float
RotatingPipe::getRotation() const
{
	if (mClockwise)
	{
		return mRotation;
	}
	else
	{
		return 3.141592654f * 2.f - mRotation;
	}
}

int
RotatingPipe::getTicksRemaining() const
{
	return mTicksRemaining;
}

void
RotatingPipe::update(float)
{
	mRotation -= RotationRate;
	if (mTicksRemaining > 0)
	{
		mTicksRemaining--;
	}
}
