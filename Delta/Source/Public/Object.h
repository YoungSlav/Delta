#pragma once

#include "stdafx.h"
#include "SharedFromThis.h"

namespace Delta
{
class Engine;

class ITickable
{
	friend class Engine;
public:
	virtual ~ITickable() = default;

protected:
	virtual void Tick(float DeltaTime) = 0;

private:
	void Update(float DeltaTime)
	{
		LifeTime += DeltaTime;
		Tick(DeltaTime);
	}

protected:
	float LifeTime = 0.0f;
};

typedef MulticastDelegate<const std::shared_ptr<class Object>> OnObjectDestroySignature;

class Object : public SharedFromThis
{
	friend class Engine;
public:
	Object(const DeltaHandle& _Handle, const std::string& _Name, const std::shared_ptr<class Engine> _Engine, const std::shared_ptr<Object> _Owner) :
		SharedFromThis(),
		Handle(_Handle),
		Name(_Name),
		EnginePtr(_Engine),
		Owner(_Owner)
	{}

	virtual ~Object();

	
	bool Initialize();
	void Destroy();

	const DeltaHandle& GetHandle() const { return Handle; }
	const std::shared_ptr<Object> GetOwner() const { return Owner.expired() ? nullptr : Owner.lock(); }
	const std::list<std::weak_ptr<Object>>& GetChildren() const { return OwnedObjects; }

	void SetName(const std::string& _Name) { Name = _Name; }
	const std::string& GetName() const { return Name; }

	template <typename Class, typename... Args>
	std::shared_ptr<Class> NewObject(const std::string& _Name = "", Args&&... args)
	{
		static_assert(std::is_base_of_v<Object, Class>);
		
		if ( !EnginePtr )
			return nullptr;
		
		DeltaHandle newHandle = NewHandle();
		std::shared_ptr<Object> newObject = std::shared_ptr<Object>(new Class(std::forward<Args>(args)... ,
			newHandle,
			_Name.empty() ? std::to_string(newHandle): _Name,
			EnginePtr, 
			Self<Object>())
		);

		if ( !InitializeNewObject(newObject) )
		{
			assert(false);
			return nullptr;
		}

		return std::dynamic_pointer_cast<Class>(newObject);
	}

	OnObjectDestroySignature OnObjectDestroyDelegate;

protected:
	virtual bool Initialize_Internal() { return true; }
	virtual void OnChildObjectAdded(std::shared_ptr<Object> _OwnedObject) {}
	virtual void OnChildObjectRemoved(std::shared_ptr<Object> _OwnedObject) {}
	virtual void OnDestroy() {}
	virtual void OnBeginPlay() {}

private:
	
	DeltaHandle NewHandle() const;
	bool InitializeNewObject(std::shared_ptr<Object> obj);

	void AddChildObject(std::shared_ptr<Object> _OwnedObject);
	void RemoveChildObject(std::shared_ptr<Object> obj);

protected:
	std::shared_ptr<class Engine> EnginePtr;

private:
	std::string Name = "";
	
	const DeltaHandle Handle = 0;

	const std::weak_ptr<Object> Owner;
	std::list<std::weak_ptr<Object>> OwnedObjects;

};

}
