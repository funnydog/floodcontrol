#pragma once

#include <span>
#include <unordered_map>
#include <vector>

#include "color.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "camera.hpp"

class Canvas;
class Font;
class Window;

class RenderTarget
{
public:
	RenderTarget();
	~RenderTarget();

	RenderTarget(const RenderTarget &) = delete;
	RenderTarget(RenderTarget &&) noexcept = delete;
	RenderTarget& operator=(const RenderTarget &) = delete;
	RenderTarget& operator=(RenderTarget &&) noexcept = delete;

	void create();
	void destroy();

	/**
	 * Get the Camera associated with the RenderTarget.
	 */
	const Camera& getCamera() const;

	/**
	 * Set the Camera associated with the RenderTarget.
	 * @param[in] view
	 */
	void setCamera(const Camera &view);

	/**
	 * Get the default Camera associated with the RenderTarget.
	 */
	const Camera& getDefaultCamera() const;

	/**
	 * Clear the target with the given @color.
	 * @param[in] color
	 */
	void clear(Color = Color::Black);

	/**
	 * Set the texture for the next primitive.
	 */
	void setTexture(const Texture *texture);

	void use(const Window &window);

	void draw(const std::string &text, glm::vec2 pos, Font &font, Color color);
	void draw(const std::string &text, const glm::mat4 &transform, Font &font, Color color);
	void draw(const Texture &texture, glm::vec2 pos, glm::vec2 size);
	void draw(const FloatRect &rect, glm::vec2 pos, glm::vec2 size, Color color=Color::White);
	void draw(const FloatRect &rect, const glm::mat4 &transform, glm::vec2 size, Color color=Color::White);
	void draw(glm::vec2 pos, glm::vec2 size, Color color);

	void beginRendering();
	void newLayer();
	void endRendering();
	void draw() const;

protected:
	void initialize();

private:
	void reserve(unsigned vcount, std::span<const std::uint16_t> indices);
private:
	Camera mDefaultCamera;
	Camera mCamera;

	struct Batch
	{
		const Texture *texture;
		unsigned vertexOffset;
		unsigned indexOffset;
		unsigned indexCount;
	};

	struct Vertex
	{
		glm::vec2 pos;
		glm::vec2 uv;
		std::uint32_t color;
	};

	std::vector<Batch> mBatches;
	std::vector<Vertex> mVertices;
	std::vector<std::uint16_t> mIndices;

	const Texture *mTexture;
	unsigned mVertexOffset;
	unsigned mVertexCount;
	unsigned mIndexOffset;
	unsigned mIndexCount;

	Texture       mWhiteTexture;
	Shader        mShader;
	unsigned      mVBO;
	unsigned      mEBO;
	unsigned      mVAO;
};
