#include "DeltaComponent.h"
#include "DeltaActor.h"

bool DeltaComponent::Initialize_Internal()
{
	if ( !DeltaObject::Initialize_Internal() )
		return false;

	if ( std::shared_ptr<DeltaObject> Owner = GetOwner() )
	{
		if ( std::shared_ptr<IDeltaTransformable> asTranformable = std::dynamic_pointer_cast<IDeltaTransformable>(Owner) )
		{
			SetParent(asTranformable);
			return true;
		}
	}
	LOG(Warning, "Owner of the component {} is not an actor! All components must be owned by an actor or another component!", GetName());
	return false;
}

std::shared_ptr<DeltaActor> DeltaComponent::GetOwningActor() const
{
	if ( std::shared_ptr<DeltaObject> Owner = GetOwner() )
	{
		if ( std::shared_ptr<DeltaActor> AsActor = std::dynamic_pointer_cast<DeltaActor>(Owner) )
		{
			return AsActor;
		}
	}
	return nullptr;
}