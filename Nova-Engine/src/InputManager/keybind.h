#pragma once

#include <functional>

// KeyBind maps a physical key input (like keyboard input, mouse click, movement and scroll) to an abstract input event.
// KeyBind wraps the specific data of the InputEvent.
// KeyBind stores a function to invoke InputManager's broadcast function.

enum class InputType {
	Press,
	Release
};

enum class InputMod {
	Ctrl,
	Shift,
	None
};

class IKeyBind {
public:
	IKeyBind(InputMod mod) :
		mod { mod }
	{}

	virtual ~IKeyBind() = 0 {};

public:
	InputMod mod;
	virtual void broadcast(InputType inputType) const = 0;
};

template <typename InputEvent>
class KeyBind : public IKeyBind {
public:
	KeyBind(InputEvent data, InputMod mod, std::function<void(InputEvent, InputType)> inputManagerBroadcastMemFn) :
		IKeyBind					{ mod },
		data						{ data },
		inputManagerBroadcastMemFn	{ inputManagerBroadcastMemFn }
	{}

private:
	void broadcast(InputType inputType) const final {
		inputManagerBroadcastMemFn(data, inputType);
	}

private:
	InputEvent data;
	std::function<void(InputEvent, InputType)> inputManagerBroadcastMemFn;
};
