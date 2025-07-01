#include "Actor.h"
#include "Engine.h"
#include "Math.h"
#include "ActorComponent.h"
#include "IRenderable.h"

using namespace Delta;

void Actor::OnChildObjectAdded(std::shared_ptr<Object> _OwnedObject)
{
	Object::OnChildObjectAdded(_OwnedObject);

	if ( std::shared_ptr<ActorComponent> oc = std::dynamic_pointer_cast<ActorComponent>(_OwnedObject) )
	{
		Components.push_back(oc);
	}
}

void Actor::OnChildObjectRemoved(std::shared_ptr<Object> _OwnedObject)
{
	Object::OnChildObjectRemoved(_OwnedObject);

	if ( std::shared_ptr<ActorComponent> oc = std::dynamic_pointer_cast<ActorComponent>(_OwnedObject) )
	{
		std::erase(Components, oc);
	}
}

void Actor::OnDestroy()
{
	Components.clear();
	Object::OnDestroy();
}

void Actor::SetVisisble(bool bNewVisible, bool bPropagateToChildren)
{
	bVisible = bNewVisible;
	if ( bPropagateToChildren )
	{
		for ( auto it : Components )
		{
			if ( std::shared_ptr<IRenderable> asRenderable = std::dynamic_pointer_cast<IRenderable>(it) )
			{
				asRenderable->SetVisisble(bNewVisible);
			}
		}
	}
}