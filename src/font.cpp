#include <codecvt>
#include <iostream>
#include <locale>
#include <stdexcept>
#include <string>

#include "rendertarget.hpp"
#include "font.hpp"

namespace
{
const int TEXTURE_WIDTH = 1024;
const int TEXTURE_HEIGHT = 1024;
const int PADDING = 2;

static inline unsigned
roundUp2(unsigned v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	return v + 1;
}
}

Font::Font()
	: mFT(nullptr)
	, mFace(nullptr)
	, mLineHeight(0)
	, mPositionX(0)
	, mPositionY(0)
	, mMaxHeight(0)
{
}

Font::~Font()
{
	FT_Done_Face(mFace);
	FT_Done_FreeType(mFT);
}

bool
Font::loadFromFile(const std::filesystem::path &path, unsigned size)
{
	FT_Done_FreeType(mFT);
	if (FT_Init_FreeType(&mFT))
	{
		std::cerr << "Font::loadFromFile() - Cannot initialize the freetype2 library"
			  << std::endl;
		return false;
	}

	FT_Done_Face(mFace);
	if (FT_New_Face(mFT, path.c_str(), 0, &mFace))
	{
		std::cerr << "Font::loadFromFile() - Failed to load the font "
			  << path << std::endl;
		return false;
	}
	FT_Set_Pixel_Sizes(mFace, 0, size);

	mLineHeight = static_cast<int>((mFace->size->metrics.ascender -
					mFace->size->metrics.descender) >> 6);
	mGlyphs.clear();
	mPositionX = mPositionY = mMaxHeight = 0;

	return true;
}

void
Font::draw(RenderTarget &target, glm::vec2 pos, const std::string &text, Color color) const
{
	if (text.empty())
	{
		return;
	}

	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
	for (auto codepoint: cv.from_bytes(text))
	{
		getGlyph(codepoint);
	}

	static const std::uint16_t indices[] = { 0, 1, 2, 1, 3, 2 };
	static const glm::vec2 unit[] = {
		{ 0.f, 0.f },
		{ 0.f, 1.f },
		{ 1.f, 0.f },
		{ 1.f, 1.f },
	};
	target.setTexture(&mTexture);
	pos.y += mLineHeight;
	for (auto codepoint: cv.from_bytes(text))
	{
		unsigned base = target.getPrimIndex(6, 4);
		target.addIndices(base, indices + 0, indices + 6);
		Vertex *vertices = target.getVertexArray(4);

		const auto &glyph = getGlyph(codepoint);
		pos.x += glyph.bearing.x;
		pos.y -= glyph.bearing.y;
		for (int i = 0; i < 4; i++)
		{
			vertices[i].pos = glyph.size * unit[i] + pos;
			vertices[i].uv = glyph.uvSize * unit[i] + glyph.uvPos;
			vertices[i].color = color;
		}
		pos.x += glyph.advance - glyph.bearing.x;
		pos.y += glyph.bearing.y;
	}
}

void
Font::draw(RenderTarget &target, const glm::mat4 &transform,
	   const std::string &text, Color color) const
{
	if (text.empty())
	{
		return;
	}

	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
	for (auto codepoint: cv.from_bytes(text))
	{
		getGlyph(codepoint);
	}

	static const std::uint16_t indices[] = { 0, 1, 2, 1, 3, 2 };
	static const glm::vec2 unit[] = {
		{ 0.f, 0.f },
		{ 0.f, 1.f },
		{ 1.f, 0.f },
		{ 1.f, 1.f },
	};
	target.setTexture(&mTexture);
	glm::vec2 pos(0.f);

	pos.y += mLineHeight;
	for (auto codepoint: cv.from_bytes(text))
	{
		unsigned base = target.getPrimIndex(6, 4);
		target.addIndices(base, indices + 0, indices + 6);
		auto vertices = target.getVertexArray(4);

		const auto &glyph = getGlyph(codepoint);
		pos.x += glyph.bearing.x;
		pos.y -= glyph.bearing.y;
		for (int i = 0; i < 4; i++)
		{
			vertices[i].pos = glm::vec2(
				transform * glm::vec4(glyph.size * unit[i] + pos, 0.f, 1.f));
			vertices[i].uv = glyph.uvSize * unit[i] + glyph.uvPos;
			vertices[i].color = color;
		}
		pos.x += glyph.advance - glyph.bearing.x;
		pos.y += glyph.bearing.y;
	}
}

glm::vec2
Font::getSize(const std::string &text) const
{
	float width = 0;
	float height = 0;
	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
	for (auto codepoint: cv.from_bytes(text))
	{
		const auto &glyph = getGlyph(codepoint);
		if (height < glyph.size.y + glyph.bearing.y)
		{
			height = glyph.size.y + glyph.bearing.y;
		}
		width += glyph.advance;
	}
	return { width, height };
}

const Texture&
Font::getTexture() const
{
	return mTexture;
}

void
Font::resizeTexture(unsigned newWidth, unsigned newHeight) const
{
	auto oldWidth = mTexture.getWidth();
	auto oldHeight = mTexture.getHeight();

	Texture newTexture;
	newTexture.create(newWidth, newHeight);
	newTexture.update(mTexture);
	std::swap(mTexture, newTexture);

	glm::vec2 scale{
		static_cast<float>(oldWidth) / newWidth,
		static_cast<float>(oldHeight) / newHeight
	};
	for (auto &[codepoint, glyph]: mGlyphs)
	{
		glyph.uvPos *= scale;
		glyph.uvSize *= scale;
	}
}

const Font::Glyph&
Font::getGlyph(char32_t codepoint) const
{
	if (const auto it = mGlyphs.find(codepoint); it != mGlyphs.end())
	{
		return it->second;
	}

	if (FT_Load_Char(mFace, codepoint, FT_LOAD_RENDER))
	{
		throw std::runtime_error(
			"Font::getGlyph() - cannot load the glyph for codepoint "
			+ std::to_string(codepoint));
	}

	int bmWidth = mFace->glyph->bitmap.width + 2 * PADDING;
	int bmHeight = mFace->glyph->bitmap.rows + 2 * PADDING;
	if (mMaxHeight < bmHeight)
	{
		mMaxHeight = bmHeight;
	}

	auto texWidth = mTexture.getWidth();
	auto texHeight = mTexture.getHeight();
	bool resize = false;
	if (unsigned right = mPositionX + bmWidth; right > texWidth)
	{
		unsigned newTexWidth = texWidth
			? texWidth * 2
			: roundUp2(right);
		if (newTexWidth <= TEXTURE_WIDTH)
		{
			texWidth = newTexWidth;
			resize = true;
		}
		else
		{
			mPositionY += mMaxHeight;
			mPositionX = 0;
			mMaxHeight = 0;
		}
	}
	if (unsigned bottom = mPositionY + bmHeight; bottom > texHeight)
	{
		unsigned newTexHeight = texHeight
			? texHeight * 2
			: roundUp2(bottom);
		if (newTexHeight <= TEXTURE_HEIGHT)
		{
			texHeight = newTexHeight;
			resize = true;
		}
		else
		{
			throw std::runtime_error("Font::getGlyph() - "
						 "no space left in the texture");
		}
	}
	if (resize)
	{
		resizeTexture(texWidth, texHeight);
	}

	// clear the pixel buffer
	mPixelBuffer.resize(bmWidth * bmHeight * 4);
	std::uint8_t *pix = mPixelBuffer.data();
	for (std::size_t i = 0; i < mPixelBuffer.size() / 4; ++i)
	{
		*pix++ = 255;
		*pix++ = 255;
		*pix++ = 255;
		*pix++ = 0;
	}

	// render the pixel
	pix = mFace->glyph->bitmap.buffer;
	for (int y = PADDING; y < bmHeight - PADDING; ++y)
	{
		for (int x = PADDING; x < bmWidth - PADDING; ++x)
		{
			const std::size_t index = x + y * bmWidth;
			mPixelBuffer[index * 4 + 3] = pix[x - PADDING];
		}
		pix += mFace->glyph->bitmap.pitch;
	}

	// upload the data
	mTexture.update(mPixelBuffer.data(), mPositionX, mPositionY, bmWidth, bmHeight);

	bmWidth -= 2 * PADDING;
	bmHeight -= 2 * PADDING;

	Glyph glyph;
	glyph.uvPos.x = static_cast<float>(mPositionX + PADDING) / texWidth;
	glyph.uvPos.y = static_cast<float>(mPositionY + PADDING) / texHeight;
	glyph.uvSize.x = static_cast<float>(bmWidth) / texWidth;
	glyph.uvSize.y = static_cast<float>(bmHeight) / texHeight;

	glyph.size = glm::vec2(bmWidth, bmHeight);
	glyph.bearing = glm::vec2(mFace->glyph->bitmap_left,
				  mFace->glyph->bitmap_top);
	glyph.advance = static_cast<float>(mFace->glyph->advance.x) / 64.f;

	const auto [it, success] = mGlyphs.insert(std::make_pair(codepoint, std::move(glyph)));
	if (!success)
	{
		throw std::runtime_error("Font::getGlyph() - "
					 "can't add the glyph to the map");
	}

	mPositionX += bmWidth + 2 * PADDING;

	return it->second;
}
