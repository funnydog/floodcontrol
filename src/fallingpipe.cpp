#include "fallingpipe.hpp"

namespace
{
static const int FallRate = 5;
}

FallingPipe::FallingPipe(Type type, int verticalOffset)
	: Pipe(type)
	, mVerticalOffset(verticalOffset)
{
}

int
FallingPipe::getVerticalOffset() const
{
	return mVerticalOffset;
}

void
FallingPipe::update(float)
{
	mVerticalOffset -= FallRate;
	if (mVerticalOffset < 0)
	{
		mVerticalOffset = 0;
	}
}
