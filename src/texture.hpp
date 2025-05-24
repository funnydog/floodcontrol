#pragma once

#include <glm/glm.hpp>

#include <filesystem>

#include "shader.hpp"

class Texture
{
public:
	bool loadFromFile(const std::filesystem::path &path);

	bool create(unsigned width, unsigned height,
	            const void *pixels=nullptr, bool repeat=false, bool smooth=true);

	void destroy();

	void update(const void *pixels);
	void update(const void *pixels, unsigned x, unsigned y, unsigned w, unsigned h);
	void update(const Texture &other, unsigned x = 0, unsigned y = 0);

	glm::vec2 getSize() const;

	unsigned getWidth() const;
	unsigned getHeight() const;

	bool isRepeated() const;
	void setRepeated(bool repeated);

	bool isSmooth() const;
	void setSmooth(bool smooth);

	void bind() const;
	void bind(int textureUnit) const noexcept;

private:
	unsigned mTexture = -1U;
};
