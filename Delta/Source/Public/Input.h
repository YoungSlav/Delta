#pragma once
#include "stdafx.h"
#include "Object.h"

namespace Delta
{

enum EKeySubscriptionType : int32
{
	Pressed = 0x01,
	Hold = 0x02,
	Released = 0x04,

	Any = 0x07
};


typedef Delegate<void, bool, float> OnKeyInputSignature;
typedef MulticastDelegate<const glm::vec2&, float> OnMouseScrollSignature;
typedef MulticastDelegate<const glm::vec2&, const glm::vec2&, float> OnMouseMoveSignature;

struct KeySubscription
{
	int32 Key = 0;
	int32 SubscribedType = EKeySubscriptionType::Any;
	OnKeyInputSignature Callback;

	bool CheckType(EKeySubscriptionType Type) const
	{
		return SubscribedType & Type;
	}
	bool CheckType(int32 Types) const
	{
		return SubscribedType & Types;
	}
};

class Input final : public Object
{
	struct KeyEvent
	{
		KeyEvent(int32 _Key, EKeySubscriptionType _Event, int32 _Mods) :
			Key(_Key), EventType(_Event), Mods(_Mods),
			bPressedNow(_Event == EKeySubscriptionType::Pressed || _Event == EKeySubscriptionType::Hold)
		{}

		int32 Key = 0;
		int32 Mods = 0;
		EKeySubscriptionType EventType;
		bool bPressedNow;
	};

public:
	template <typename... Args>
	Input(Args&&... args) :
		Object(std::forward<Args>(args)...)
	{
	}
	
	void SetMouseEnabled(bool bNewMouseEnabled) const;
	void ProcessInput(float DeltaTime);

	bool GetKeyState(int32 Key) const;
	const glm::vec2& GetMousePosition() const { return MousePos; }

	void SubscribeKey(const KeySubscription& NewSubscription);
	OnMouseScrollSignature OnMouseScrollDelegate;
	OnMouseMoveSignature OnMouseMoveDelegate;

	void UnSubscribeAll(std::shared_ptr<Object> Owner);
	void UnSubscribeKey(int32 Key, std::shared_ptr<Object> Owner);

protected:
	virtual bool Initialize_Internal() override;
	virtual void OnDestroy() override;

	void OnInputKey(int32 Key, int32 Action);
	void OnMouseMove(glm::vec2 MousePos);
	void OnMouseScroll(glm::vec2 MouseScroll);

private:
	std::vector<KeySubscription> KeysSubscribers;
		
	std::set<int32> PressedKeys;
	std::set<int32> OldPressedStates;


	glm::vec2 MousePos = glm::vec2(0.0);
	glm::vec2 DeltaMouseMove = glm::vec2(0.0);
	glm::vec2 DeltaMouseScroll = glm::vec2(0.0);
};

}