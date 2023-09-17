#include <iostream>
#include <cassert>

#include <GL/glew.h>

#include "color.hpp"
#include "glcheck.hpp"
#include "rendertarget.hpp"
#include "window.hpp"

namespace
{
const char *vertexShader =
	"\n#version 330 core"
	"\nlayout (location = 0) in vec2 Position;"
	"\nlayout (location = 1) in vec2 UV;"
	"\nlayout (location = 2) in vec4 Color;"
	"\nuniform mat4 Projection;"
	"\nout vec2 FragUV;"
	"\nout vec4 FragColor;"
	"\nvoid main()"
	"\n{"
	"\n	FragUV = UV;"
	"\n	FragColor = Color;"
	"\n	gl_Position = Projection * vec4(Position, 0, 1);"
	"\n}";

const char *fragmentShader =
	"\n#version 330 core"
	"\nin vec2 FragUV;"
	"\nin vec4 FragColor;"
	"\nuniform sampler2D Texture;"
	"\nlayout (location = 0) out vec4 OutColor;"
	"\nvoid main()"
	"\n{"
	"\n	OutColor = FragColor * texture(Texture, FragUV.st);"
	"\n}";

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
	mShader.attach(vertexShader, ShaderType::Vertex);
	mShader.attach(fragmentShader, ShaderType::Fragment);
	mShader.link();

	glm::vec2 size = window.getSize();
	mDefaultCamera.setCenter(size * 0.5f);
	mDefaultCamera.setSize(size);
	mCamera = mDefaultCamera;

	glCheck(glEnable(GL_CULL_FACE));
	glCheck(glEnable(GL_BLEND));
	glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	glCheck(glGenVertexArrays(1, &mVAO));
	glCheck(glGenBuffers(1, &mVBO));
	glCheck(glGenBuffers(1, &mEBO));
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

	Shader::bind(&mShader);
	ShaderUniform projection = mShader.getUniform("Projection");
	ShaderUniform sampler = mShader.getUniform("Texture");

	sampler.set(textureUnit);
	projection.set(mCamera.getTransform());

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
			Texture::bind(channel->texture, textureUnit);
		}

		// draw
		glCheck(glDrawElementsBaseVertex(
				GL_TRIANGLES,
				channel->idxBuffer.size(),
				GL_UNSIGNED_SHORT,
				reinterpret_cast<GLvoid*>(channel->idxOffset),
				channel->vtxOffset));
	}

	glCheck(glDisableVertexAttribArray(2));
	glCheck(glDisableVertexAttribArray(1));
	glCheck(glDisableVertexAttribArray(0));

	glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));

	Shader::bind(nullptr);

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
