#include "rectangle.hpp"

Rectangle::Rectangle(glm::vec2 size, Color color)
	: mSize(size)
	, mColor(color)
{
}

glm::vec2
Rectangle::getSize() const
{
	return mSize;
}

void
Rectangle::setSize(glm::vec2 size)
{
	mSize = size;
}

Color
Rectangle::getColor() const
{
	return mColor;
}

void
Rectangle::setColor(Color color)
{
	mColor = color;
}
