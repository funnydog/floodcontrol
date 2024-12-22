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
	: mIsBatching(false)
	, mChannelList(nullptr)
	, mChannelTail(&mChannelList)
	, mCurrent(nullptr)
	, mFreeChannels(nullptr)
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

	beginBatch();
	DrawChannel *channel = mFreeChannels;
	while (channel)
	{
		DrawChannel *next = channel->next;
		delete channel;
		channel = next;
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

	glCheck(glEnable(GL_CULL_FACE));
	glCheck(glEnable(GL_BLEND));
	glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        // VBO and EBO
	glCheck(glGenBuffers(1, &mVBO));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
	glCheck(glGenBuffers(1, &mEBO));

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
RenderTarget::addLayer()
{
	// NOTE: by deleting the texture->channel association
	// we force to build another set of channels.
	mChannelMap.clear();
}

void
RenderTarget::beginBatch()
{
	mVertices.clear();
	mChannelMap.clear();
	*mChannelTail = mFreeChannels;
	mFreeChannels = mChannelList;
	mChannelList = mCurrent = nullptr;
	mChannelTail = &mChannelList;
}

void
RenderTarget::endBatch()
{
	mIndices.clear();
	for (auto channel = mChannelList; channel; channel = channel->next)
	{
		channel->idxOffset = mIndices.size() * sizeof(std::uint16_t);
		mIndices.insert(mIndices.end(),
				channel->idxBuffer.begin(),
				channel->idxBuffer.end());
	}
}

void
RenderTarget::draw()
{
	assert(mVAO && mVBO && mEBO && "OpenGL objects not initialized.");
	const int textureUnit = 0;

	if (!mChannelList)
	{
		return;
	}

	if (mIsBatching)
	{
		mIsBatching = false;
		endBatch();
	}

	glCheck(glBindVertexArray(mVAO));

	mShader.use();
	mShader.getUniform("projection").setMatrix4(mCamera.getTransform());
	mShader.getUniform("image").setInteger(0);

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

	const Texture *currentTexture = nullptr;
	for (auto channel = mChannelList; channel; channel = channel->next)
	{
		// skip empty channels
		if (!channel->texture || channel->idxBuffer.empty())
		{
			continue;
		}

		// dont bind against the same texture
		if (currentTexture != channel->texture)
		{
			currentTexture = channel->texture;
			currentTexture->bind(textureUnit);
		}

		// draw
		glCheck(glDrawElementsBaseVertex(
				GL_TRIANGLES,
				channel->idxBuffer.size(),
				GL_UNSIGNED_SHORT,
				reinterpret_cast<GLvoid*>(channel->idxOffset),
				channel->vtxOffset));
	}

	glCheck(glBindVertexArray(0));
}

RenderTarget::DrawChannel *
RenderTarget::newChannel(const Texture *texture, unsigned vtxOffset)
{
	DrawChannel *channel;
	if (mFreeChannels)
	{
		channel = mFreeChannels;
		channel->idxBuffer.clear();
		mFreeChannels = channel->next;
	}
	else
	{
		channel = new DrawChannel();
	}

	// channel initialization
	channel->texture = texture;
	channel->vtxOffset = vtxOffset;
	channel->idxOffset = 0;
	channel->next = nullptr;

	// update the channel list
	*mChannelTail = channel;
	mChannelTail = &channel->next;

	return channel;
}

void
RenderTarget::setTexture(const Texture *texture)
{
	// switch to batching state if needed
	if (!mIsBatching)
	{
		mIsBatching = true;
		beginBatch();
	}

	// the null texture means a white texture;
	if (!texture)
	{
		texture = &mWhiteTexture;
	}

	// return early if the texture is the same
	if (mCurrent && mCurrent->texture == texture)
	{
		return;
	}

	// look for a channel with the same texture
	if (auto it = mChannelMap.find(texture); it != mChannelMap.end())
	{
		// channel found
		mCurrent = it->second;
	}
	else
	{
		// or add a new one
		mCurrent = newChannel(texture, mVertices.size());
		mChannelMap[texture] = mCurrent;
	}
}

std::uint16_t
RenderTarget::getPrimIndex(unsigned idxCount, unsigned vtxCount)
{
	// ensure we have a current channel and the rendertarget is in
	// batching state.
	if (!mIsBatching)
	{
		mIsBatching = true;
		beginBatch();
		mCurrent = newChannel(&mWhiteTexture, 0);
	}

	// ensure we have enough space for the indices
	unsigned index = mVertices.size() - mCurrent->vtxOffset;
	if (index + vtxCount > UINT16_MAX)
	{
		mCurrent = newChannel(mCurrent->texture, mVertices.size());
		mChannelMap[mCurrent->texture] = mCurrent;
		index = 0;
	}

	// reserve the space for the vertices and the indices
	mVertices.reserve(mVertices.size() + vtxCount);
	mCurrent->idxBuffer.reserve(mCurrent->idxBuffer.size() + idxCount);

	return index;
}

Vertex*
RenderTarget::getVertexArray(unsigned vtxCount)
{
	auto size = mVertices.size();
	mVertices.resize(size + vtxCount);
	return &mVertices[size];
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
		unsigned base = getPrimIndex(6, 4);
		addIndices(base, indices+0, indices+6);
		Vertex *vertices = getVertexArray(4);

		const auto &glyph = font.getGlyph(codepoint);
		pos.x += glyph.bearing.x;
		pos.y -= glyph.bearing.y;
		for (int i = 0; i < 4; i++)
		{
			vertices[i].pos = glyph.size * units[i] + pos;
			vertices[i].uv = glyph.uvSize * units[i] + glyph.uvPos;
			vertices[i].color = color;
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
		unsigned base = getPrimIndex(6, 4);
		addIndices(base, indices+0, indices+6);
		Vertex *vertices = getVertexArray(4);

		const auto &glyph = font.getGlyph(codepoint);
		pos.x += glyph.bearing.x;
		pos.y -= glyph.bearing.y;
		for (int i = 0; i < 4; i++)
		{
			vertices[i].pos = glm::vec2(
				transform * glm::vec4(glyph.size * units[i] + pos, 0.f, 1.f));
			vertices[i].uv = glyph.uvSize * units[i] + glyph.uvPos;
			vertices[i].color = color;
		}
		pos.x += glyph.advance - glyph.bearing.x;
		pos.y += glyph.bearing.y;
	}
}
