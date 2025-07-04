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
		KeyEvent(int32 _Key, EKeySubscriptionType _Event, int32 _Mods) :
			key(_Key), eventType(_Event), mods(_Mods),
			bPressedNow(_Event == EKeySubscriptionType::PRESSED || _Event == EKeySubscriptionType::HOLD)
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
	std::vector<KeySubscription> keysSubscribers;
		
	std::set<int32> pressedKeys;
	std::set<int32> oldPressedStates;


	glm::vec2 mousePos = glm::vec2(0.0);
	glm::vec2 deltaMouseMove = glm::vec2(0.0);
	glm::vec2 deltaMouseScroll = glm::vec2(0.0);
};

}