#pragma once

#include "pipe.hpp"

class RotatingPipe: public Pipe
{
public:
	RotatingPipe(Type type, bool clockwise);

	float getRotation() const;
	int getTicksRemaining() const;

	void update(float dt);

private:
	float mRotation;
	bool  mClockwise;
	int   mTicksRemaining;
};
