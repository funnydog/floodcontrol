#pragma once

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include "resources.hpp"

#include "view.hpp"

class ViewStack
{
public:
	explicit ViewStack(const Context &context);

	template<typename View, typename... Args>
	void registerView(ViewID view, Args&&... args);

	bool update(float dt);
	bool handleEvent(const Event &event);
	void render(RenderTarget &target);

	void pushView(ViewID viewID);
	void popView();
	void clearStack();

	bool empty() const;

private:
	enum Action
	{
		Push,
		Pop,
		Clear,
	};
	struct PendingChange
	{
		Action action;
		ViewID viewID;
	};
	View::Ptr createState(ViewID viewID);
	void applyPendingChanges();

private:
	Context mContext;
	std::vector<View::Ptr> mStack;
	std::vector<PendingChange> mPendingChanges;
	std::unordered_map<ViewID, std::function<View::Ptr()>> mFactories;
};

template<typename View, typename... Args>
void
ViewStack::registerView(ViewID view, Args&&... args)
{
	// NOTE: don't capture the args by reference
	mFactories[view] = [this, args...]() {
		return std::make_unique<View>(*this, mContext, args...);
	};
}
