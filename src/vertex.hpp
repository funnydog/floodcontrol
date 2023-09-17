#pragma once

#include <cstdint>
#include <glm/glm.hpp>

struct Vertex
{
	glm::vec2 pos;
	glm::vec2 uv;
	std::uint32_t color;
};
