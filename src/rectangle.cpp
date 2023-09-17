#include "rectangle.hpp"
#include "rendertarget.hpp"

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

void
Rectangle::draw(RenderTarget &target, glm::vec2 position) const
{
	const glm::vec2 units[4] = {
		{ 0.f, 0.f },
		{ 0.f, 1.f },
		{ 1.f, 0.f },
		{ 1.f, 1.f },
	};

	Vertex vertices[4];
	for (int i = 0; i < 4; i++)
	{
		vertices[i].pos = units[i] * mSize + position;
		vertices[i].uv = units[i];
		vertices[i].color = mColor;
	}

	target.setTexture(nullptr);
	auto base = target.getPrimIndex(6, 4);
	const std::uint16_t indices[] = { 0, 1, 2, 1, 3, 2 };
	target.addIndices(base, indices + 0, indices + 6);
	target.addVertices(vertices + 0, vertices + 4);
}
