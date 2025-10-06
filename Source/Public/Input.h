#pragma once
#include "stdafx.h"
#include "Object.h"

namespace Delta
{

enum EKeySubscriptionType : int32
{
	PRESSED = 0x01,
	HOLD = 0x02,
	RELEASED = 0x04,

	ANY = 0x07
};


typedef Delegate<void, bool, float> OnKeyInputSignature;
typedef MulticastDelegate<const glm::vec2&, float> OnMouseScrollSignature;
typedef MulticastDelegate<const glm::vec2&, const glm::vec2&, float> OnMouseMoveSignature;

struct KeySubscription
{
	int32 key = 0;
	int32 subscribedType = EKeySubscriptionType::ANY;
	OnKeyInputSignature callback;

	bool checkType(EKeySubscriptionType Type) const
	{
		return subscribedType & Type;
	}
	bool checkType(int32 Types) const
	{
		return subscribedType & Types;
	}
};

class Input final : public Object
{
	struct KeyEvent
	{
		KeyEvent(int32 inKey, EKeySubscriptionType inEvent, int32 inMods) :
			key(inKey), eventType(inEvent), mods(inMods),
			bPressedNow(inEvent == EKeySubscriptionType::PRESSED || inEvent == EKeySubscriptionType::HOLD)
		{}

		int32 key = 0;
		int32 mods = 0;
		EKeySubscriptionType eventType;
		bool bPressedNow;
	};

public:
	template <typename... Args>
	Input(Args&&... args) :
		Object(std::forward<Args>(args)...)
	{
	}
	
	void setMouseEnabled(bool bNewMouseEnabled) const;
	void processInput(float DeltaTime);

	bool getKeyState(int32 Key) const;
	const glm::vec2& getMousePosition() const { return mousePos; }

	void subscribeKey(const KeySubscription& NewSubscription);
	OnMouseScrollSignature OnMouseScrollDelegate;
	OnMouseMoveSignature OnMouseMoveDelegate;

	void unsubscribeAll(std::shared_ptr<Object> Owner);
	void unsubscribeKey(int32 Key, std::shared_ptr<Object> Owner);

protected:
	virtual bool initialize_Internal() override;
	virtual void onDestroy() override;

	void onInputKey(int32 Key, int32 Action);
	void onMouseMove(glm::vec2 MousePos);
	void onMouseScroll(glm::vec2 MouseScroll);

private:
	inline const std::string getKeyName(int32 key)
	{
		switch (key)
		{
			case GLFW_KEY_ESCAPE: return "Escape";
			case GLFW_KEY_ENTER: return "Enter";
			case GLFW_KEY_TAB: return "Tab";
			case GLFW_KEY_BACKSPACE: return "Backspace";
			case GLFW_KEY_INSERT: return "Insert";
			case GLFW_KEY_DELETE: return "Delete";
			case GLFW_KEY_RIGHT: return "Right";
			case GLFW_KEY_LEFT: return "Left";
			case GLFW_KEY_DOWN: return "Down";
			case GLFW_KEY_UP: return "Up";
			case GLFW_KEY_PAGE_UP: return "PageUp";
			case GLFW_KEY_PAGE_DOWN: return "PageDown";
			case GLFW_KEY_HOME: return "Home";
			case GLFW_KEY_END: return "End";
			case GLFW_KEY_CAPS_LOCK: return "CapsLock";
			case GLFW_KEY_SCROLL_LOCK: return "ScrollLock";
			case GLFW_KEY_NUM_LOCK: return "NumLock";
			case GLFW_KEY_PRINT_SCREEN: return "PrintScreen";
			case GLFW_KEY_PAUSE: return "Pause";
			case GLFW_KEY_SPACE: return "Space";
			case GLFW_KEY_LEFT_SHIFT: return "LeftShift";
			case GLFW_KEY_LEFT_CONTROL: return "LeftCtrl";
			case GLFW_KEY_LEFT_ALT: return "LeftAlt";
			case GLFW_KEY_LEFT_SUPER: return "LeftSuper";
			case GLFW_KEY_RIGHT_SHIFT: return "RightShift";
			case GLFW_KEY_RIGHT_CONTROL: return "RightCtrl";
			case GLFW_KEY_RIGHT_ALT: return "RightAlt";
			case GLFW_KEY_RIGHT_SUPER: return "RightSuper";
			case GLFW_KEY_MENU: return "Menu";
			case GLFW_MOUSE_BUTTON_RIGHT: return "MouseRight";
			case GLFW_MOUSE_BUTTON_LEFT: return "MouseLeft";
			case GLFW_MOUSE_BUTTON_MIDDLE: return "MouseMiddle";
			default:
				if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_F25)
				{
					return "F" + std::to_string(key - GLFW_KEY_F1 + 1);
				}
				else
				{
					int scancode = glfwGetKeyScancode(key);
					const char* name = glfwGetKeyName(key, scancode);
					return name ? std::string(name) : std::to_string(key);
				}
		}
	} 

private:
	std::vector<KeySubscription> keysSubscribers;
		
	std::set<int32> pressedKeys;
	std::set<int32> oldPressedStates;


	glm::vec2 mousePos = glm::vec2(0.0);
	glm::vec2 deltaMouseMove = glm::vec2(0.0);
	glm::vec2 deltaMouseScroll = glm::vec2(0.0);
};

}
