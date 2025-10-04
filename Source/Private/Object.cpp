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
	LOG(Log, "Object {} destroyed! Total object count: {}", name, ObjectsSpawned);
#else
	LOG(Log, "'{}' destroyed", getName());
#endif
}

bool Object::initializeNewObject(std::shared_ptr<Object> newObject)
{
	if ( !newObject->initialize() )
		return false;

	addChildObject(newObject);
	engine->registerObject(newObject);

	return true;
}

DeltaHandle Object::newHandle() const
{
	return engine->generateNewHandle();
}


bool Object::initialize()
{

#if	TEST_MEMORY_LEAKS
	ObjectsSpawned++;
	LOG(Log, "Object {} created! Total object count: {}", name, ObjectsSpawned);
#endif

	return initialize_Internal();
}

void Object::addChildObject(std::shared_ptr<Object> inOwnedObject)
{	
	inOwnedObject->OnObjectDestroyDelegate.AddSP(Self<Object>(), &Object::removeChildObject);

	ownedObjects.push_back(inOwnedObject);
	onChildObjectAdded(inOwnedObject);
}

void Object::removeChildObject(std::shared_ptr<Object> obj)
{
	ownedObjects.remove_if([&](const std::weak_ptr<Object>& weakObj)
	{
		if (auto shared = weakObj.lock())
			return shared.get() == obj.get();
		return false;
	});
	onChildObjectRemoved(obj);
}

void Object::destroy()
{
	LOG(Log, "Destroy object: '{}'", getName());
	LOG_INDENT

	onDestroy();

	OnObjectDestroyDelegate.Broadcast(Self<Object>());

	std::list<std::weak_ptr<Object>> OwnedObjectsTmp = ownedObjects;
	for( auto it : OwnedObjectsTmp )
	{
		if ( it.expired() )
			continue;
		it.lock()->destroy();
	}
	ownedObjects.clear();

	if ( engine )
	{
		engine->destroyObject(Self<Object>());
		engine.reset();
	}
	LOG(Log, "'{}' cleaned up.", getName());
	
}