#include "Actor.h"
#include "Engine.h"
#include "Math.h"
#include "ActorComponent.h"
#include "IRenderable.h"

using namespace Delta;

void Actor::onChildObjectAdded(std::shared_ptr<Object> _OwnedObject)
{
	Object::onChildObjectAdded(_OwnedObject);

	if ( std::shared_ptr<ActorComponent> oc = std::dynamic_pointer_cast<ActorComponent>(_OwnedObject) )
	{
		components.push_back(oc);
	}
}

void Actor::onChildObjectRemoved(std::shared_ptr<Object> _OwnedObject)
{
	Object::onChildObjectRemoved(_OwnedObject);

	if ( std::shared_ptr<ActorComponent> oc = std::dynamic_pointer_cast<ActorComponent>(_OwnedObject) )
	{
		std::erase(components, oc);
	}
}

void Actor::onDestroy()
{
	components.clear();
	Object::onDestroy();
}

void Actor::setVisisble(bool bNewVisible, bool bPropagateToChildren)
{
	bVisible = bNewVisible;
	if ( bPropagateToChildren )
	{
		for ( auto it : components )
		{
			if ( std::shared_ptr<IRenderable> asRenderable = std::dynamic_pointer_cast<IRenderable>(it) )
			{
				asRenderable->setVisisble(bNewVisible);
			}
		}
	}
}