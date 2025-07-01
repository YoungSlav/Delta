#include "ActorComponent.h"
#include "Actor.h"

using namespace Delta;

bool ActorComponent::Initialize_Internal()
{
	if ( !Object::Initialize_Internal() )
		return false;

	if ( std::shared_ptr<Object> Owner = GetOwner() )
	{
		if ( std::shared_ptr<ITransformable> asTranformable = std::dynamic_pointer_cast<ITransformable>(Owner) )
		{
			SetParent(asTranformable);
			return true;
		}
	}
	LOG(Warning, "Owner of the component {} is not an actor! All components must be owned by an actor or another component!", GetName());
	return false;
}

std::shared_ptr<Actor> ActorComponent::GetOwningActor() const
{
	if ( std::shared_ptr<Object> Owner = GetOwner() )
	{
		if ( std::shared_ptr<Actor> AsActor = std::dynamic_pointer_cast<Actor>(Owner) )
		{
			return AsActor;
		}
	}
	return nullptr;
}