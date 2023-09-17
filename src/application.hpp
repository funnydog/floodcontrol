#pragma once

#include "eventqueue.hpp"
#include "window.hpp"
#include "rendertarget.hpp"
#include "resources.hpp"
#include "resourceholder.hpp"
#include "viewstack.hpp"
#include "font.hpp"
#include "texture.hpp"

class Application
{
public:
	Application();
	~Application();

	void run();

private:
	void processInput();
	void loadAssets();
	void registerViews();

private:
	EventQueue    mEventQueue;
	Window        mWindow;
	RenderTarget  mTarget;
	FontHolder    mFonts;
	TextureHolder mTextures;
	ViewStack     mViewStack;
};
