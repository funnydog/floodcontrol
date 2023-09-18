#include "scorezoom.hpp"

namespace
{
static const int mMaxDisplayCount = 30;
static const float mScaleAmount = 0.4f;
}

ScoreZoom::ScoreZoom(const std::string &text, Color color)
	: text(text)
	, drawColor(color)
	, mDisplayCounter(0)
{
}

float
ScoreZoom::getScale() const
{
	return mScaleAmount * mDisplayCounter;
}

bool
ScoreZoom::isCompleted() const
{
	return mDisplayCounter > mMaxDisplayCount;
}

void
ScoreZoom::update(float)
{
	mDisplayCounter++;
}
