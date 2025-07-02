#include "stdafx.h"
#include "Input.h"
#include "Engine.h"
#include "Window.h"
#include "DeltaMath.h"

using namespace Delta;

bool Input::Initialize_Internal()
{
	Object::Initialize_Internal();

	EnginePtr->GetWindow()->OnInputKeyDelegate.AddSP(Self<Input>(), &Input::OnInputKey);
	EnginePtr->GetWindow()->OnMouseMoveDelegate.AddSP(Self<Input>(), &Input::OnMouseMove);
	EnginePtr->GetWindow()->OnMouseScrollDelegate.AddSP(Self<Input>(), &Input::OnMouseScroll);

	return true;
}

void Input::OnDestroy()
{
	Object::OnDestroy();

	if ( EnginePtr->GetWindow() )
	{
		EnginePtr->GetWindow()->OnInputKeyDelegate.RemoveObject(this);
		EnginePtr->GetWindow()->OnMouseMoveDelegate.RemoveObject(this);
		EnginePtr->GetWindow()->OnMouseScrollDelegate.RemoveObject(this);
	}
}

void Input::OnInputKey(int32 Key, int32 Action)
{
	if ( Action == GLFW_PRESS || Action == GLFW_REPEAT )
		PressedKeys.insert(Key);
	else
		PressedKeys.erase(Key);
}
void Input::OnMouseMove(glm::vec2 NewMousePos)
{
	DeltaMouseMove += NewMousePos - MousePos;
	MousePos = NewMousePos;
}

void Input::OnMouseScroll(glm::vec2 MouseScroll)
{
	DeltaMouseScroll = MouseScroll;
}

void Input::SetMouseEnabled(bool bNewMouseEnabled) const
{
	EnginePtr->GetWindow()->SetMouseEnabled(bNewMouseEnabled);
}

void Input::SubscribeKey(const KeySubscription& NewSubscription)
{
	KeysSubscribers.push_back(NewSubscription);
}

void Input::UnSubscribeAll(std::shared_ptr<Object> Owner)
{
	for (auto it = KeysSubscribers.begin(); it != KeysSubscribers.end();) 
	{
		if ( it->Callback.IsBoundTo(Owner.get()) )
			it = KeysSubscribers.erase(it);
		else 
			++it;
	}
	OnMouseScrollDelegate.RemoveObject(Owner.get());
	OnMouseMoveDelegate.RemoveObject(Owner.get());

}
void Input::UnSubscribeKey(int32 Key, std::shared_ptr<Object> Owner)
{
	for (auto it = KeysSubscribers.begin(); it != KeysSubscribers.end();) 
	{
		if ( Key == it->Key && it->Callback.IsBoundTo(Owner.get()) ) 
			it = KeysSubscribers.erase(it);
		else 
			++it;
	}
}

bool Input::GetKeyState(int32 Key) const
{
	return PressedKeys.find(Key) != PressedKeys.end();
}

void Input::ProcessInput(float DeltaTime)
{
	for ( KeySubscription& listener : KeysSubscribers )
	{
		bool bPressedNow = PressedKeys.find(listener.Key) != PressedKeys.end();
		bool bPressedOld = OldPressedStates.find(listener.Key) != OldPressedStates.end();

		if ( !bPressedNow && bPressedOld )
		{
			// button released
			if ( listener.CheckType(EKeySubscriptionType::Released) )
			{
				// user subscribed to button being released
				listener.Callback.Execute(bPressedNow, DeltaTime);
			}
		}
		else if ( bPressedNow && !bPressedOld )
		{
			// button pressed
			if ( listener.CheckType(EKeySubscriptionType::Pressed | EKeySubscriptionType::Hold) )
			{
				// user subscribed to button being pressed or hold
				listener.Callback.Execute(bPressedNow, DeltaTime);
			}
		}
		else if( bPressedNow && bPressedOld )
		{
			// button hold
			if ( listener.CheckType(EKeySubscriptionType::Hold) )
			{
				// user subscribed to button being hold
				listener.Callback.Execute(bPressedNow, DeltaTime);
			}
		}
	}
	OldPressedStates = PressedKeys;

	
	if ( !Math::IsNearlyZero(DeltaMouseMove) )
	{
		OnMouseMoveDelegate.Broadcast(MousePos, DeltaMouseMove, DeltaTime);
	}

	if ( !Math::IsNearlyZero(DeltaMouseScroll) )
	{
		OnMouseScrollDelegate.Broadcast(DeltaMouseScroll, DeltaTime);
	}

	DeltaMouseScroll = glm::vec2(0.0);
	DeltaMouseMove = glm::vec2(0.0);
}

