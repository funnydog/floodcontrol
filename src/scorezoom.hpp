#pragma once

#include <string>

#include "color.hpp"

class ScoreZoom
{
public:
	ScoreZoom(const std::string &text, Color color);

	float getScale() const;
	bool isCompleted() const;

	void update(float dt);

public:
	std::string text;
	Color drawColor;

private:
	int mDisplayCounter;
};
