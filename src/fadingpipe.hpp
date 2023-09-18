#pragma once

#include "pipe.hpp"

class FadingPipe: public Pipe
{
public:
	FadingPipe(Type type, bool filled);

	float getAlphaLevel() const;

	void update(float dt);

private:
	float mAlphaLevel;
};
