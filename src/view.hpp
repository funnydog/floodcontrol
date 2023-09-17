#pragma once

#include <memory>

#include "event.hpp"
#include "resources.hpp"

class Window;
class RenderTarget;

struct Context
{
	Window        *window;
	RenderTarget  *target;
	FontHolder    *fonts;
	TextureHolder *textures;
};

class View
{
public:
	using Ptr = std::unique_ptr<View>;

public:
	virtual ~View() = default;

	/**
	 * Update the view state.
	 *
	 * @param[in] dt Elapsed time in seconds.
	 *
	 * @retval true don't update underlying view.
	 * @retval false update underlying view.
	 */
	virtual bool update(float dt) = 0;

	/**
	 * Handle the passed event.
	 *
	 * @param[in] event Reference to the received event.
	 *
	 * @retval true event handled.
	 * @retval false event not handled.
	 */
	virtual bool handleEvent(const Event &event) = 0;

	/**
	 * Render the view using the @target.
	 *
	 * @param[in] target Reference to a RenderTarget class.
	 */
	virtual void render(RenderTarget &target) = 0;
};
