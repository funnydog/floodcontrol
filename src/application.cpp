#include <cstdint>
#include <stdexcept>

#include <GLFW/glfw3.h>

#include "application.hpp"

#include "titleview.hpp"
#include "gameview.hpp"
#include "gameoverview.hpp"
#include "pauseview.hpp"

namespace
{
const unsigned ScreenWidth = 800;
const unsigned ScreenHeight = 600;
}

Application::Application()
	: mEventQueue()
	, mWindow()
	, mTarget()
	, mFonts()
	, mTextures()
	, mViewStack({ &mWindow, &mTarget, &mFonts, &mTextures, })
{
	if (!glfwInit())
	{
		const char *error;
		glfwGetError(&error);
		throw std::runtime_error(error);
	}

	mWindow.open("FloodControl", ScreenWidth, ScreenHeight);

	// track the window events
	mEventQueue.track(mWindow);

	// tell the target to render on the window
	mTarget.create();
	mTarget.use(mWindow);

	// with a context in use we load the assets
	loadAssets();

	registerViews();

	// push the first view
	mViewStack.pushView(ViewID::Title);
	mViewStack.update(0.f);
}

Application::~Application()
{
	mTextures.destroy();
	mTarget.destroy();
	glfwTerminate();
}

void
Application::loadAssets()
{
	mFonts.load(FontID::Pericles36, "assets/fonts/Peric.ttf", 36);

	mTextures.load(TextureID::Background, "assets/textures/background.png");
	mTextures.load(TextureID::TileSheet, "assets/textures/tile_sheet.png");
	mTextures.load(TextureID::TitleScreen, "assets/textures/title_screen.png");
}

void
Application::registerViews()
{
	mViewStack.registerView<TitleView>(ViewID::Title);
	mViewStack.registerView<GameView>(ViewID::GamePlay);
	mViewStack.registerView<GameOverView>(ViewID::GameOver);
	mViewStack.registerView<PauseView>(ViewID::Paused);
}

void
Application::run()
{
	// variable-time game loop
	auto currentTime = glfwGetTime();
	while (!mWindow.isClosed() && !mViewStack.empty())
	{
		auto newTime = glfwGetTime();
		auto frameTime = newTime - currentTime;
		currentTime = newTime;

		processInput();
		mViewStack.update(frameTime);

		// render
		mViewStack.render(mTarget);
		mWindow.display();
	}
}

void
Application::processInput()
{
	mEventQueue.poll();
	Event event;
	while (mEventQueue.pop(event))
	{
		if (mViewStack.handleEvent(event))
		{
			// event handled by a view in the stack
		}
		else if (const auto ep(std::get_if<KeyPressed>(&event)); ep
			 && ep->key == GLFW_KEY_ESCAPE)
		{
			mWindow.close();
		}
	}
}
