#include "DeltaObject.h"
#include "DeltaEngine.h"

#if TEST_MEMORY_LEAKS
int32 ObjectsSpawned = 0;
#endif

DeltaObject::~DeltaObject()
{
#if TEST_MEMORY_LEAKS
	ObjectsSpawned--;
	LOG(Warning, "Object {} destroyed! Total object count: {}", Name, ObjectsSpawned);
#endif
}

bool DeltaObject::InitializeNewObject(std::shared_ptr<DeltaObject> newObject)
{
	if ( !newObject->Initialize() )
		return false;

	AddChildObject(newObject);
	Engine->RegisterObject(newObject);

	return true;
}

DeltaHandle DeltaObject::NewHandle() const
{
	return Engine->GenerateNewHandle();
}


bool DeltaObject::Initialize()
{

#if	TEST_MEMORY_LEAKS
	ObjectsSpawned++;
	LOG(Warning, "Object {} created! Total object count: {}", Name, ObjectsSpawned);
#endif

	return Initialize_Internal();
}

void DeltaObject::AddChildObject(std::shared_ptr<DeltaObject> _OwnedObject)
{	
	_OwnedObject->OnObjectDestroyDelegate.AddSP(Self<DeltaObject>(), &DeltaObject::RemoveChildObject);

	OwnedObjects.push_back(_OwnedObject);
	OnChildObjectAdded(_OwnedObject);
}

void DeltaObject::RemoveChildObject(std::shared_ptr<DeltaObject> obj)
{
	OwnedObjects.remove_if([&](const std::weak_ptr<DeltaObject>& weakObj)
	{
		if (auto shared = weakObj.lock())
			return shared.get() == obj.get();
		return false;
    });
	OnChildObjectRemoved(obj);
}

void DeltaObject::Destroy()
{
	OnObjectDestroyDelegate.Broadcast(Self<DeltaObject>());

	for( auto it : OwnedObjects )
	{
		if ( it.expired() )
			continue;
		it.lock()->Destroy();
	}
	OwnedObjects.clear();

	if ( Engine )
	{
		OnDestroy();
		Engine->DestroyObject(Self<DeltaObject>());
		Engine.reset();
	}
}