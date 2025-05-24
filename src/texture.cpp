#include <cassert>
#include <iostream>

#include <GL/glew.h>

#include "glcheck.hpp"
#include "texture.hpp"
#include "stb_image.h"

bool
Texture::create(unsigned width, unsigned height, const void *pixels, bool repeat, bool smooth)
{
	if (width == 0 || height == 0)
	{
		std::cerr << "Failed to create the texture, invalid size ("
			  << width << ", " << height << ")" << std::endl;
		return false;
	}

	if (mTexture == -1U)
	{
		glCheck(glGenTextures(1, &mTexture));
	}

	glCheck(glBindTexture(GL_TEXTURE_2D, mTexture));
	GLint parameter = repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;
	glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, parameter));
	glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, parameter));

	parameter = smooth ? GL_LINEAR : GL_NEAREST;
	glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, parameter));
	glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, parameter));

	glCheck(glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height));
	if (pixels)
	{
		glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
		                        static_cast<GLsizei>(width),
		                        static_cast<GLsizei>(height),
		                        GL_RGBA, GL_UNSIGNED_BYTE, pixels));
	}
	return true;
}

void
Texture::destroy()
{
	if (mTexture != -1U)
	{
		glCheck(glDeleteTextures(1, &mTexture));
		mTexture = -1U;
	}
}

void
Texture::update(const void *pixels)
{
	update(pixels, 0, 0, getWidth(), getHeight());
}

void
Texture::update(const void *pixels, unsigned x, unsigned y, unsigned w, unsigned h)
{
	assert(x + w <= getWidth() && "Destination x coordinate is outside of the texture");
	assert(y + h <= getHeight() && "Destination y coordinate is outside of the texture");

	if (pixels == nullptr || mTexture == -1U)
	{
		return;
	}

	glCheck(glBindTexture(GL_TEXTURE_2D, mTexture));
	glCheck(glTexSubImage2D(
			GL_TEXTURE_2D,
			0,
			static_cast<GLint>(x),
			static_cast<GLint>(y),
			static_cast<GLsizei>(w),
			static_cast<GLsizei>(h),
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			pixels));
	glCheck(glBindTexture(GL_TEXTURE_2D, 0));
	glCheck(glFlush());
}

void
Texture::update(const Texture &other, unsigned x, unsigned y)
{
	auto dstWidth = getWidth();
	auto dstHeight = getHeight();
	auto srcWidth = other.getWidth();
	auto srcHeight = other.getHeight();
	assert(x + srcWidth <= dstWidth
	       && "Destination x coordinate is outside of the texture");
	assert(y + srcHeight <= dstHeight
	       && "Destination y coordinate is outside of the texture");

	if (mTexture == -1U || other.mTexture == -1U)
	{
		return;
	}

	GLint oldReadFB, oldDrawFB;
	glCheck(glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &oldReadFB));
	glCheck(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &oldDrawFB));

	GLuint sourceFB, destFB;
	glCheck(glGenFramebuffers(1, &sourceFB));
	glCheck(glGenFramebuffers(1, &destFB));

	if (!sourceFB || !destFB)
	{
		throw std::runtime_error("Texture::update() - cannot create framebuffer objects");
	}

	glCheck(glBindFramebuffer(GL_READ_FRAMEBUFFER, sourceFB));
	glCheck(glFramebufferTexture2D(GL_READ_FRAMEBUFFER,
				       GL_COLOR_ATTACHMENT0,
				       GL_TEXTURE_2D,
				       other.mTexture,
				       0));

	glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destFB));
	glCheck(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
				       GL_COLOR_ATTACHMENT0,
				       GL_TEXTURE_2D,
				       mTexture,
				       0));

	GLenum sourceStatus = glCheckFramebufferStatus(GL_READ_FRAMEBUFFER);
	GLenum destStatus = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if (sourceStatus != GL_FRAMEBUFFER_COMPLETE
	    && destStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error("Framebuffers not complete");
	}

	glCheck(glBlitFramebuffer(
			0, 0, srcWidth, srcHeight,
			x, y, x + srcWidth, x + srcHeight,
			GL_COLOR_BUFFER_BIT,
			GL_NEAREST));

	glCheck(glBindFramebuffer(GL_READ_FRAMEBUFFER, oldReadFB));
	glCheck(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, oldDrawFB));

	glCheck(glDeleteFramebuffers(1, &destFB));
	glCheck(glDeleteFramebuffers(1, &sourceFB));
}

bool
Texture::loadFromFile(const std::filesystem::path &path)
{
	int width, height, channels;
	auto *pixels = stbi_load(path.c_str(), &width, &height, &channels, 4);
	if (!pixels)
	{
		std::cerr << "Texture::loadFromFile - Cannot load " << path.string()
			  << " (" << stbi_failure_reason() << ")."
			  << std::endl;
		return false;
	}
	bool result = create(width, height, pixels);
	stbi_image_free(pixels);
	return result;
}

glm::vec2
Texture::getSize() const
{
	glm::vec2 size(0);
	if (mTexture != -1U)
	{
		GLint width, height;
		glCheck(glBindTexture(GL_TEXTURE_2D, mTexture));
		glCheck(glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width));
		glCheck(glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height));
		glCheck(glBindTexture(GL_TEXTURE_2D, 0));
		size.x = static_cast<float>(width);
		size.y = static_cast<float>(height);
	}
	return size;
}

unsigned
Texture::getWidth() const
{
	GLint width = 0;
	if (mTexture != -1U)
	{
		glCheck(glBindTexture(GL_TEXTURE_2D, mTexture));
		glCheck(glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width));
	}
	return width;
}

unsigned
Texture::getHeight() const
{
	GLint height = 0;
	if (mTexture != -1U)
	{
		glCheck(glBindTexture(GL_TEXTURE_2D, mTexture));
		glCheck(glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height));
	}
	return height;
}

bool
Texture::isRepeated() const
{
	GLint glWrapping = GL_CLAMP_TO_EDGE;
	if (mTexture != -1U)
	{
		glCheck(glBindTexture(GL_TEXTURE_2D, mTexture));
		glCheck(glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &glWrapping));
	}
	return glWrapping == GL_REPEAT;
}

void
Texture::setRepeated(bool repeated)
{
	assert(mTexture != -1U && "Texture not created");

	GLint glWrapping = repeated ? GL_REPEAT : GL_CLAMP_TO_EDGE;
	glCheck(glBindTexture(GL_TEXTURE_2D, mTexture));
	glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glWrapping));
	glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glWrapping));
	glCheck(glBindTexture(GL_TEXTURE_2D, 0));
}

bool
Texture::isSmooth() const
{
	GLint glFiltering = GL_NEAREST;
	if (mTexture != -1U)
	{
		glCheck(glBindTexture(GL_TEXTURE_2D, mTexture));
		glCheck(glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &glFiltering));
	}
	return glFiltering == GL_LINEAR;
}

void
Texture::setSmooth(bool smooth)
{
	assert(mTexture != -1U && "Texture not created");

	GLint glFiltering = smooth ? GL_LINEAR : GL_NEAREST;
	glCheck(glBindTexture(GL_TEXTURE_2D, mTexture));
	glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glFiltering));
	glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glFiltering));
	glCheck(glBindTexture(GL_TEXTURE_2D, 0));
}

void
Texture::bind() const
{
	glCheck(glBindTexture(GL_TEXTURE_2D, mTexture));
}

void
Texture::bind(int textureUnit) const noexcept
{
	glCheck(glActiveTexture(GL_TEXTURE0 + textureUnit));
	glCheck(glBindTexture(GL_TEXTURE_2D, mTexture));
}
