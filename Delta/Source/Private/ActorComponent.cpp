#include "ActorComponent.h"
#include "Actor.h"

using namespace Delta;

bool ActorComponent::initialize_Internal()
{
	if ( !Object::initialize_Internal() )
		return false;

	if ( std::shared_ptr<Object> Owner = getOwner() )
	{
		if ( std::shared_ptr<ITransformable> asTranformable = std::dynamic_pointer_cast<ITransformable>(Owner) )
		{
			setParent(asTranformable);
			return true;
		}
	}
	LOG(Warning, "Owner of the component {} is not an actor! All components must be owned by an actor or another component!", getName());
	return false;
}

std::shared_ptr<Actor> ActorComponent::getOwningActor() const
{
	if ( std::shared_ptr<Object> Owner = getOwner() )
	{
		if ( std::shared_ptr<Actor> AsActor = std::dynamic_pointer_cast<Actor>(Owner) )
		{
			return AsActor;
		}
	}
	return nullptr;
}