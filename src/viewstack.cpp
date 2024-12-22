#include <cassert>

#include "viewstack.hpp"
#include "rendertarget.hpp"

ViewStack::ViewStack(const Context &context)
	: mContext(context)
{
}

bool
ViewStack::update(float dt)
{
	bool handled = false;
	for (auto it = mStack.rbegin(), end = mStack.rend();
	     it != end;
	     ++it)
	{
		handled = (*it)->update(dt);
		if (handled)
		{
			break;
		}
	}
	applyPendingChanges();
	return handled;
}

bool
ViewStack::handleEvent(const Event &event)
{
	bool handled = false;
	for (auto it = mStack.rbegin(), end = mStack.rend();
	     it != end;
	     ++it)
	{
		handled = (*it)->handleEvent(event);
		if (handled)
		{
			break;
		}
	}
	applyPendingChanges();
	return handled;
}

void
ViewStack::render(RenderTarget &target)
{
	for (auto &view: mStack)
	{
		target.beginRendering();
		view->render(target);
		target.endRendering();
		target.draw();
	}
}

void
ViewStack::pushView(ViewID viewID)
{
	mPendingChanges.push_back({Push, viewID});
}

void
ViewStack::popView()
{
	mPendingChanges.push_back({Pop, ViewID::None});
}

void
ViewStack::clearStack()
{
	mPendingChanges.push_back({Clear, ViewID::None});
}

bool
ViewStack::empty() const
{
	return mStack.empty();
}

View::Ptr
ViewStack::createState(ViewID viewID)
{
	auto it = mFactories.find(viewID);
	assert(it != mFactories.end() && "View not found");
	return it->second();
}

void
ViewStack::applyPendingChanges()
{
	for (const auto &change: mPendingChanges)
	{
		switch (change.action)
		{
		case Push:
			mStack.push_back(createState(change.viewID));
			break;

		case Pop:
			mStack.pop_back();
			break;

		case Clear:
			mStack.clear();
			break;
		}
	}
	mPendingChanges.clear();
}
