#pragma once

#include "stdafx.h"
#include "SharedFromThis.h"

namespace Delta
{
class Engine;

class ITickable
{
	friend class Scene;
public:
	virtual ~ITickable() = default;

protected:
	virtual void tick(float DeltaTime) = 0;

private:
	void update(float DeltaTime)
	{
		lifeTime += DeltaTime;
		tick(DeltaTime);
	}

protected:
	float lifeTime = 0.0f;
};

typedef MulticastDelegate<const std::shared_ptr<class Object>> OnObjectDestroySignature;

class Object : public SharedFromThis
{
	friend class Engine;
public:
	Object(const DeltaHandle& inHandle, const std::string& inName, const std::shared_ptr<class Engine> inEngine, const std::shared_ptr<Object> inOwner) :
		SharedFromThis(),
		handle(inHandle),
		name(inName),
		engine(inEngine),
		owner(inOwner)
	{}

	virtual ~Object();

	
	bool initialize();
	void destroy();

	const DeltaHandle& getHandle() const { return handle; }
	const std::shared_ptr<Object> getOwner() const { return owner.expired() ? nullptr : owner.lock(); }
	const std::list<std::weak_ptr<Object>>& getChildren() const { return ownedObjects; }

	void setName(const std::string& inName) { name = inName; }
	const std::string& getName() const { return name; }

	template <typename Class, typename... Args>
	std::shared_ptr<Class> spawn(const std::string& inName = "", Args&&... args)
	{
		static_assert(std::is_base_of_v<Object, Class>);
		
		if ( !engine )
			return nullptr;

		LOG(Log, "Create object: '{}'", inName);
		LOG_INDENT
		
		DeltaHandle h = newHandle();
		std::shared_ptr<Object> newObject = std::shared_ptr<Object>(new Class(std::forward<Args>(args)... ,
			h,
			inName.empty() ? std::to_string(h): inName,
			engine, 
			Self<Object>())
		);

		if ( !initializeNewObject(newObject) )
		{
			LOG(Error, "Failed to construct object: '{}'", inName);
			return nullptr;
		}

		LOG(Log, "Object '{}' initialized", inName);
		return std::dynamic_pointer_cast<Class>(newObject);
	}

	OnObjectDestroySignature OnObjectDestroyDelegate;

protected:
	virtual bool initialize_Internal() { return true; }
	virtual void onChildObjectAdded(std::shared_ptr<Object> inOwnedObject) {}
	virtual void onChildObjectRemoved(std::shared_ptr<Object> inOwnedObject) {}
	virtual void onDestroy() {}
	virtual void onBeginPlay() {}

private:
	
	DeltaHandle newHandle() const;
	bool initializeNewObject(std::shared_ptr<Object> obj);

	void addChildObject(std::shared_ptr<Object> inOwnedObject);
	void removeChildObject(std::shared_ptr<Object> obj);

protected:
	std::shared_ptr<class Engine> engine;

private:
	std::string name = "";
	
	const DeltaHandle handle = 0;

	const std::weak_ptr<Object> owner;
	std::list<std::weak_ptr<Object>> ownedObjects;

};

}
