#pragma once

#include <unordered_map>
#include <vector>

#include "color.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "vertex.hpp"
#include "camera.hpp"

class Canvas;
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
	 * Force a new draw command.
	 */
	void addLayer();

	/**
	 * Send the blob of vertices to the GPU.
	 */
	void draw();

	/**
	 * Set the texture for the next primitive.
	 */
	void setTexture(const Texture *texture);

	/**
	 * Get the base index for the primitive and reserve space in
	 * the vertex and index buffers.
	 */
	std::uint16_t getPrimIndex(unsigned idxCount, unsigned vtxCount);

	/**
	 * Add a sequence of indices to the DrawChannel, with an
	 * @offset applied to them. The offset is obtained by using
	 * getPrimIndex().
	 *
	 * @param[in] offset offset to add to each index.
	 * @param[in] start first iterator of the range.
	 * @param[in] end end iterator of the range.
	 */
	template <typename Iterator>
	void addIndices(std::uint16_t offset, Iterator start, Iterator end);

	/**
	 * Add a sequence of vertices to the DrawChannel.
	 *
	 * @param[in] start first iterator of the range.
	 * @param[in] end end iterator of the range.
	 */
	template <typename Iterator>
	void addVertices(Iterator start, Iterator end);

	Vertex* getVertexArray(unsigned vtxCount);

	/**
	 * Use the @window as a drawing backend.
	 *
	 * @param[in] window Window to use as a drawing backend.
	 */
	void use(const Window &window);

protected:
	void initialize();

private:
	struct DrawChannel
	{
		const Texture *texture;
		unsigned vtxOffset;
		unsigned idxOffset;
		std::vector<std::uint16_t> idxBuffer;
		DrawChannel *next;
	};

private:
	DrawChannel *newChannel(const Texture *texture, unsigned vtxOffset);
	void beginBatch();
	void endBatch();

private:
	Camera mDefaultCamera;
	Camera mCamera;

	std::vector<Vertex>        mVertices;
	std::vector<std::uint16_t> mIndices;
	std::unordered_map<const Texture *, DrawChannel*> mChannelMap;

	bool          mIsBatching;
	DrawChannel  *mChannelList;
	DrawChannel **mChannelTail;
	DrawChannel  *mCurrent;
	DrawChannel  *mFreeChannels;

	Texture       mWhiteTexture;
	Shader        mShader;
	unsigned      mVBO;
	unsigned      mEBO;
	unsigned      mVAO;
};

template <typename Iterator>
void
RenderTarget::addIndices(std::uint16_t offset, Iterator start, Iterator end)
{
	for (; start != end; ++start)
	{
		mCurrent->idxBuffer.push_back(offset + *start);
	}
}

template <typename Iterator>
void
RenderTarget::addVertices(Iterator start, Iterator end)
{
	mVertices.insert(mVertices.end(), start, end);
}
