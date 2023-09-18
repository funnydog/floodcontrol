#include "fadingpipe.hpp"

namespace
{
static const float AlphaChangeRate = 0.02f;
}

FadingPipe::FadingPipe(Type type, bool filled)
	: Pipe(type, filled)
	, mAlphaLevel(1.f)
{
}

float
FadingPipe::getAlphaLevel() const
{
	return mAlphaLevel;
}

void
FadingPipe::update(float)
{
	mAlphaLevel -= AlphaChangeRate;
	if (mAlphaLevel < 0.f)
	{
		mAlphaLevel = 0.f;
	}
}
