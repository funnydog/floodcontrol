#pragma once

#include <glm/glm.hpp>

#include "rect.hpp"

class Camera
{
public:
	Camera();
	Camera(glm::vec2 center, glm::vec2 size);

	glm::vec2 getCenter() const;
	void setCenter(glm::vec2 center);

	glm::vec2 getSize() const;
	void setSize(glm::vec2 size);

	float getRotation() const;
	void setRotation(float rotation);

	void setViewport(const FloatRect &viewport);

	void move(glm::vec2 offset);
	void rotate(float angle);
	void scale(float factor);

	const glm::mat4& getTransform() const;
	const glm::mat4& getInverse() const;

private:
	glm::vec2 mCenter;
	glm::vec2 mSize;
	float     mRotation;
	FloatRect mViewport;

	// cached elements
	mutable glm::mat4 mTransform;
	mutable glm::mat4 mInverse;
	mutable bool      mTransformNeedsUpdate;
	mutable bool      mInverseNeedsUpdate;
};
