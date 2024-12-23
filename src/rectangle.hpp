#pragma once

#include <glm/glm.hpp>

#include "color.hpp"

class Rectangle
{
public:
	explicit Rectangle(glm::vec2 size = glm::vec2(0.f),
	                   Color = Color::White);

	glm::vec2 getSize() const;
	void setSize(glm::vec2 size);

	Color getColor() const;
	void setColor(Color color);

private:
	glm::vec2 mSize;
	Color mColor;
};
