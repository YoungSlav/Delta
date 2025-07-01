#include "DeltaActor.h"
#include "DeltaEngine.h"
#include "DeltaMath.h"
#include "DeltaComponent.h"
#include "IDeltaRenderable.h"


void DeltaActor::OnChildObjectAdded(std::shared_ptr<DeltaObject> _OwnedObject)
{
	DeltaObject::OnChildObjectAdded(_OwnedObject);

	if ( std::shared_ptr<DeltaComponent> oc = std::dynamic_pointer_cast<DeltaComponent>(_OwnedObject) )
	{
		Components.push_back(oc);
	}
}

void DeltaActor::OnChildObjectRemoved(std::shared_ptr<DeltaObject> _OwnedObject)
{
	DeltaObject::OnChildObjectRemoved(_OwnedObject);

	if ( std::shared_ptr<DeltaComponent> oc = std::dynamic_pointer_cast<DeltaComponent>(_OwnedObject) )
	{
		std::erase(Components, oc);
	}
}

void DeltaActor::OnDestroy()
{
	Components.clear();
	DeltaObject::OnDestroy();
}

void DeltaActor::SetVisisble(bool bNewVisible, bool bPropagateToChildren)
{
	bVisible = bNewVisible;
	if ( bPropagateToChildren )
	{
		for ( auto it : Components )
		{
			if ( std::shared_ptr<IDeltaRenderable> asRenderable = std::dynamic_pointer_cast<IDeltaRenderable>(it) )
			{
				asRenderable->SetVisisble(bNewVisible);
			}
		}
	}
}