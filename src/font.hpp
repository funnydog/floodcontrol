#pragma once

#include <filesystem>
#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "color.hpp"
#include "texture.hpp"

class Font
{
public:
	struct Glyph
	{
		glm::vec2 uvPos;
		glm::vec2 uvSize;
		glm::vec2 size;
		glm::vec2 bearing;
		float advance;
	};

	Font();
	~Font();

	bool loadFromFile(const std::filesystem::path &path, unsigned size);

	glm::vec2 getSize(const std::string &text) const;

	const Texture &getTexture() const;
	const Glyph &getGlyph(char32_t codepoint) const;
	float getLineHeight() const;

private:
	void resizeTexture(unsigned newWidth, unsigned newHeight) const;

private:
	mutable std::unordered_map<char32_t, Glyph> mGlyphs;
	mutable std::vector<std::uint8_t> mPixelBuffer;
	mutable Texture mTexture;
	FT_Library mFT;
	mutable FT_Face mFace;
	int mLineHeight;
	mutable int mPositionX;
	mutable int mPositionY;
	mutable int mMaxHeight;
};
