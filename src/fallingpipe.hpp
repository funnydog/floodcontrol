#pragma once

#include "pipe.hpp"

class FallingPipe: public Pipe
{
public:
	FallingPipe(Type type, int verticalOffset);

	int getVerticalOffset() const;

	void update(float dt);

private:
	int mVerticalOffset;
};
