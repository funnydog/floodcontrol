#include <iostream>
#include <cassert>
#include <codecvt>

#include <GL/glew.h>

#include "color.hpp"
#include "font.hpp"
#include "glcheck.hpp"
#include "rendertarget.hpp"
#include "window.hpp"

namespace
{
static const std::uint16_t indices[] = { 0, 1, 2, 1, 3, 2 };
static const glm::vec2 units[] = {
	{ 0.f, 0.f },
	{ 0.f, 1.f },
	{ 1.f, 0.f },
	{ 1.f, 1.f },
};
}

RenderTarget::RenderTarget()
	: mTexture(nullptr)
	, mVertexOffset(0)
	, mVertexCount(0)
	, mIndexOffset(0)
	, mIndexCount(0)
	, mVBO(0)
	, mEBO(0)
	, mVAO(0)
{
}

RenderTarget::~RenderTarget()
{
	mShader.destroy();
	if (mVAO)
	{
		glCheck(glBindVertexArray(0));
		glCheck(glDeleteVertexArrays(1, &mVAO));
	}
	if (mEBO)
	{
		glCheck(glDeleteBuffers(1, &mEBO));
	}
	if (mVBO)
	{
		glCheck(glDeleteBuffers(1, &mVBO));
	}
}

void
RenderTarget::use(const Window &window)
{
	mWhiteTexture.create(1, 1, &Color::White);
	mShader.create();
	if (!mShader.attachFile(Shader::Type::Vertex, "assets/shaders/pos_uv_color.vs")
	    || !mShader.attachFile(Shader::Type::Fragment, "assets/shaders/uv_color.fs")
	    || !mShader.link())
	{
		throw std::runtime_error("RenderTarget::use() - shader error");
	}

	glm::vec2 size = window.getSize();
	mDefaultCamera.setCenter(size * 0.5f);
	mDefaultCamera.setSize(size);
	mCamera = mDefaultCamera;

	mShader.use();
	mShader.getUniform("projection").setMatrix4(mCamera.getTransform());
	mShader.getUniform("image").setInteger(0);

	glCheck(glEnable(GL_CULL_FACE));
	glCheck(glEnable(GL_BLEND));
	glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        // VBO and EBO
	glCheck(glGenBuffers(1, &mEBO));
	glCheck(glGenBuffers(1, &mVBO));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, mVBO));

	// VAO
	glCheck(glGenVertexArrays(1, &mVAO));
	glCheck(glBindVertexArray(mVAO));
	glCheck(glEnableVertexAttribArray(0));
	glCheck(glVertexAttribPointer(
			0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
			reinterpret_cast<GLvoid*>(offsetof(Vertex, pos))));
	glCheck(glEnableVertexAttribArray(1));
	glCheck(glVertexAttribPointer(
			1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
			reinterpret_cast<GLvoid*>(offsetof(Vertex, uv))));
	glCheck(glEnableVertexAttribArray(2));
	glCheck(glVertexAttribPointer(
			2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex),
			reinterpret_cast<GLvoid*>(offsetof(Vertex, color))));
	glCheck(glBindVertexArray(0));
}

const Camera&
RenderTarget::getDefaultCamera() const
{
	return mDefaultCamera;
}

const Camera&
RenderTarget::getCamera() const
{
	return mCamera;
}

void
RenderTarget::setCamera(const Camera &view)
{
	mCamera = view;
}

void
RenderTarget::clear(Color color)
{
	glm::vec4 clearColor(color);
	glCheck(glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a));
	glCheck(glClear(GL_COLOR_BUFFER_BIT));
}

void
RenderTarget::beginRendering()
{
	mBatches.clear();
	mVertices.clear();
	mIndices.clear();
	mTexture = &mWhiteTexture;
	mVertexOffset = mVertexCount = 0;
	mIndexOffset = mIndexCount = 0;
}

void
RenderTarget::newLayer()
{
	endRendering();
}

void
RenderTarget::endRendering()
{
	mBatches.emplace_back(mTexture, mVertexOffset,
	                      mIndexOffset*sizeof(mIndices[0]),
	                      mIndexCount-mIndexOffset);
	mVertexOffset = mVertexCount;
	mIndexOffset = mIndexCount;
}

void
RenderTarget::setTexture(const Texture *texture)
{
	if (texture == nullptr)
	{
		texture = &mWhiteTexture;
	}
	if (texture != mTexture && mIndexCount > mIndexOffset)
	{
		endRendering();
	}
	mTexture = texture;
}

void
RenderTarget::reserve(unsigned vcount, std::span<const std::uint16_t> indices)
{
	auto base = mVertexCount - mVertexOffset;
	if (base + vcount > UINT16_MAX)
	{
		endRendering();
		base = 0;
	}
	mVertexCount += vcount;
	for (auto i : indices)
	{
		mIndices.push_back(base + i);
	}
	mIndexCount += indices.size();
}

void
RenderTarget::draw() const
{
	mShader.use();

	glCheck(glBindVertexArray(mVAO));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
	glCheck(glBufferData(GL_ARRAY_BUFFER,
	                     mVertices.size() * sizeof(mVertices[0]),
	                     mVertices.data(),
	                     GL_STREAM_DRAW));

	glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO));
	glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
	                     mIndices.size() * sizeof(mIndices[0]),
	                     mIndices.data(),
	                     GL_STREAM_DRAW));

	for (const auto &batch : mBatches)
	{
		batch.texture->bind(0);
		glCheck(glDrawElementsBaseVertex(
			        GL_TRIANGLES,
			        batch.indexCount,
			        GL_UNSIGNED_SHORT,
			        reinterpret_cast<GLvoid*>(batch.indexOffset),
			        batch.vertexOffset));
	}

	glCheck(glBindVertexArray(0));
}

void
RenderTarget::draw(const std::string &text, glm::vec2 pos, Font &font, Color color)
{
	if (text.empty())
	{
		return;
	}

	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
	for (auto codepoint : cv.from_bytes(text))
	{
		font.getGlyph(codepoint);
	}

	setTexture(&font.getTexture());
	pos.y += font.getLineHeight();
	for (auto codepoint : cv.from_bytes(text))
	{
		reserve(4, indices);

		const auto &glyph = font.getGlyph(codepoint);
		pos.x += glyph.bearing.x;
		pos.y -= glyph.bearing.y;
		for (auto unit : units)
		{
			Vertex v;
			v.pos = glyph.size * unit + pos;
			v.uv = glyph.uvSize * unit + glyph.uvPos;
			v.color = color;
			mVertices.push_back(v);
		}
		pos.x += glyph.advance - glyph.bearing.x;
		pos.y += glyph.bearing.y;
	}
}

void
RenderTarget::draw(const std::string &text, const glm::mat4 &transform, Font &font, Color color)
{
	if (text.empty())
	{
		return;
	}

	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
	for (auto codepoint : cv.from_bytes(text))
	{
		font.getGlyph(codepoint);
	}

	setTexture(&font.getTexture());

	glm::vec2 pos(0.f);
	pos.y += font.getLineHeight();
	for (auto codepoint : cv.from_bytes(text))
	{
		reserve(4, indices);
		const auto &glyph = font.getGlyph(codepoint);
		pos.x += glyph.bearing.x;
		pos.y -= glyph.bearing.y;
		for (auto unit : units)
		{
			Vertex v;
			v.pos = glm::vec2(
				transform * glm::vec4(glyph.size * unit + pos, 0.f, 1.f));
			v.uv = glyph.uvSize * unit + glyph.uvPos;
			v.color = color;
			mVertices.push_back(v);
		}
		pos.x += glyph.advance - glyph.bearing.x;
		pos.y += glyph.bearing.y;
	}
}

void
RenderTarget::draw(const Texture &texture, glm::vec2 pos, glm::vec2 size)
{
	setTexture(&texture);
	reserve(4, indices);
	for (auto unit : units)
	{
		Vertex v;
		v.pos = unit * size + pos;
		v.uv = unit;
		v.color = Color::White;
		mVertices.push_back(v);
	}
}

void
RenderTarget::draw(glm::vec2 pos, glm::vec2 size, Color color)
{
	setTexture(&mWhiteTexture);
	reserve(4, indices);
	for (auto unit : units)
	{
		Vertex v;
		v.pos = unit * size + pos;
		v.uv = unit;
		v.color = color;
		mVertices.push_back(v);
	}
}

void
RenderTarget::draw(const FloatRect &rect, glm::vec2 pos, glm::vec2 size, Color color)
{
	reserve(4, indices);
	for (auto unit : units)
	{
		Vertex v;
		v.pos = unit * size + pos;
		v.uv = unit * rect.size + rect.pos;
		v.color = color;
		mVertices.push_back(v);
	}
}

void
RenderTarget::draw(const FloatRect &rect, const glm::mat4 &transform, glm::vec2 size, Color color)
{
	reserve(4, indices);
	for (auto unit : units)
	{
		Vertex v;
		v.pos = glm::vec2(transform * glm::vec4(unit * size, 0.f, 1.f));
		v.uv = unit * rect.size + rect.pos;
		v.color = color;
		mVertices.push_back(v);
	}
}
