#include "Object.h"
#include "Engine.h"

using namespace Delta;

#if TEST_MEMORY_LEAKS
int32 ObjectsSpawned = 0;
#endif

Object::~Object()
{
#if TEST_MEMORY_LEAKS
	ObjectsSpawned--;
	LOG(Warning, "Object {} destroyed! Total object count: {}", Name, ObjectsSpawned);
#endif
}

bool Object::InitializeNewObject(std::shared_ptr<Object> newObject)
{
	if ( !newObject->Initialize() )
		return false;

	AddChildObject(newObject);
	EnginePtr->RegisterObject(newObject);

	return true;
}

DeltaHandle Object::NewHandle() const
{
	return EnginePtr->GenerateNewHandle();
}


bool Object::Initialize()
{

#if	TEST_MEMORY_LEAKS
	ObjectsSpawned++;
	LOG(Warning, "Object {} created! Total object count: {}", Name, ObjectsSpawned);
#endif

	return Initialize_Internal();
}

void Object::AddChildObject(std::shared_ptr<Object> _OwnedObject)
{	
	_OwnedObject->OnObjectDestroyDelegate.AddSP(Self<Object>(), &Object::RemoveChildObject);

	OwnedObjects.push_back(_OwnedObject);
	OnChildObjectAdded(_OwnedObject);
}

void Object::RemoveChildObject(std::shared_ptr<Object> obj)
{
	OwnedObjects.remove_if([&](const std::weak_ptr<Object>& weakObj)
	{
		if (auto shared = weakObj.lock())
			return shared.get() == obj.get();
		return false;
    });
	OnChildObjectRemoved(obj);
}

void Object::Destroy()
{
	OnObjectDestroyDelegate.Broadcast(Self<Object>());

	for( auto it : OwnedObjects )
	{
		if ( it.expired() )
			continue;
		it.lock()->Destroy();
	}
	OwnedObjects.clear();

	if ( EnginePtr )
	{
		OnDestroy();
		EnginePtr->DestroyObject(Self<Object>());
		EnginePtr.reset();
	}
}