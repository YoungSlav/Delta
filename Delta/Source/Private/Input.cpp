#include "stdafx.h"
#include "Input.h"
#include "Engine.h"
#include "Window.h"
#include "DeltaMath.h"

using namespace Delta;

bool Input::initialize_Internal()
{
	Object::initialize_Internal();

	LOG(Log, "Creating input handler");

	engine->getWindow()->OnInputKeyDelegate.AddSP(Self<Input>(), &Input::onInputKey);
	engine->getWindow()->OnMouseMoveDelegate.AddSP(Self<Input>(), &Input::onMouseMove);
	engine->getWindow()->OnMouseScrollDelegate.AddSP(Self<Input>(), &Input::onMouseScroll);

	return true;
}

void Input::onDestroy()
{
	Object::onDestroy();

	if ( engine->getWindow() )
	{
		engine->getWindow()->OnInputKeyDelegate.RemoveObject(this);
		engine->getWindow()->OnMouseMoveDelegate.RemoveObject(this);
		engine->getWindow()->OnMouseScrollDelegate.RemoveObject(this);
	}
}

void Input::onInputKey(int32 Key, int32 Action)
{
	if ( Action == GLFW_PRESS || Action == GLFW_REPEAT )
		pressedKeys.insert(Key);
	else
		pressedKeys.erase(Key);
}
void Input::onMouseMove(glm::vec2 NewMousePos)
{
	deltaMouseMove += NewMousePos - mousePos;
	mousePos = NewMousePos;
}

void Input::onMouseScroll(glm::vec2 MouseScroll)
{
	deltaMouseScroll = MouseScroll;
}

void Input::setMouseEnabled(bool bNewMouseEnabled) const
{
	engine->getWindow()->setMouseEnabled(bNewMouseEnabled);
}

void Input::subscribeKey(const KeySubscription& NewSubscription)
{
	LOG(Log, "Input subscribe key {}", NewSubscription.key);
	keysSubscribers.push_back(NewSubscription);
}

void Input::unsubscribeAll(std::shared_ptr<Object> Owner)
{

	LOG(Log, "Input unsubscribe all");

	for (auto it = keysSubscribers.begin(); it != keysSubscribers.end();) 
	{
		if ( it->callback.IsBoundTo(Owner.get()) )
			it = keysSubscribers.erase(it);
		else 
			++it;
	}
	OnMouseScrollDelegate.RemoveObject(Owner.get());
	OnMouseMoveDelegate.RemoveObject(Owner.get());

}
void Input::unsubscribeKey(int32 Key, std::shared_ptr<Object> Owner)
{
	LOG(Log, "Input unsubscribe key {} for {}", Key, Owner->getName());

	for (auto it = keysSubscribers.begin(); it != keysSubscribers.end();) 
	{
		if ( Key == it->key && it->callback.IsBoundTo(Owner.get()) ) 
			it = keysSubscribers.erase(it);
		else 
			++it;
	}
}

bool Input::getKeyState(int32 Key) const
{
	return pressedKeys.find(Key) != pressedKeys.end();
}

void Input::processInput(float DeltaTime)
{
	for ( KeySubscription& listener : keysSubscribers )
	{
		bool bPressedNow = pressedKeys.find(listener.key) != pressedKeys.end();
		bool bPressedOld = oldPressedStates.find(listener.key) != oldPressedStates.end();

		if ( !bPressedNow && bPressedOld )
		{
			// button released
			if ( listener.checkType(EKeySubscriptionType::RELEASED) )
			{
				// user subscribed to button being released
				listener.callback.Execute(bPressedNow, DeltaTime);
			}
		}
		else if ( bPressedNow && !bPressedOld )
		{
			// button pressed
			if ( listener.checkType(EKeySubscriptionType::PRESSED | EKeySubscriptionType::HOLD) )
			{
				// user subscribed to button being pressed or hold
				listener.callback.Execute(bPressedNow, DeltaTime);
			}
		}
		else if( bPressedNow && bPressedOld )
		{
			// button hold
			if ( listener.checkType(EKeySubscriptionType::HOLD) )
			{
				// user subscribed to button being hold
				listener.callback.Execute(bPressedNow, DeltaTime);
			}
		}
	}
	oldPressedStates = pressedKeys;

	
	if ( !Math::isNearlyZero(deltaMouseMove) )
	{
		OnMouseMoveDelegate.Broadcast(mousePos, deltaMouseMove, DeltaTime);
	}

	if ( !Math::isNearlyZero(deltaMouseScroll) )
	{
		OnMouseScrollDelegate.Broadcast(deltaMouseScroll, DeltaTime);
	}

	deltaMouseScroll = glm::vec2(0.0);
	deltaMouseMove = glm::vec2(0.0);
}

