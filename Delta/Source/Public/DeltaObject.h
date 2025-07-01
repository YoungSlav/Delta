#pragma once

#include "stdafx.h"
#include "SharedFromThis.h"


class DeltaEngine;

class IDeltaTickable
{
	friend class DeltaEngine;
public:
	virtual ~IDeltaTickable() = default;

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

typedef MulticastDelegate<const std::shared_ptr<class DeltaObject>> OnObjectDestroySignature;

class DeltaObject : public SharedFromThis
{

	friend class DeltaEngine;
public:
	DeltaObject(const DeltaHandle& _Handle, const std::string& _Name, const std::shared_ptr<class DeltaEngine> _Engine, const std::shared_ptr<DeltaObject> _Owner) :
		SharedFromThis(),
		Handle(_Handle),
		Name(_Name),
		Engine(_Engine),
		Owner(_Owner)
	{}

	virtual ~DeltaObject();

	
	bool Initialize();
	void Destroy();

	const DeltaHandle& GetHandle() const { return Handle; }
	const std::shared_ptr<DeltaObject> GetOwner() const { return Owner.expired() ? nullptr : Owner.lock(); }
	const std::list<std::weak_ptr<DeltaObject>>& GetChildren() const { return OwnedObjects; }

	void SetName(const std::string& _Name) { Name = _Name; }
	const std::string& GetName() const { return Name; }

	template <typename Class, typename... Args>
	std::shared_ptr<Class> NewObject(const std::string& _Name = "", Args&&... args)
	{
		static_assert(std::is_base_of_v<DeltaObject, Class>);
		
		if ( !Engine )
			return nullptr;
		
		DeltaHandle newHandle = NewHandle();
		std::shared_ptr<DeltaObject> newObject = std::shared_ptr<DeltaObject>(new Class(std::forward<Args>(args)... ,
			newHandle,
			_Name.empty() ? std::to_string(newHandle): _Name,
			Engine, 
			Self<DeltaObject>())
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
	virtual void OnChildObjectAdded(std::shared_ptr<DeltaObject> _OwnedObject) {}
	virtual void OnChildObjectRemoved(std::shared_ptr<DeltaObject> _OwnedObject) {}
	virtual void OnDestroy() {}
	virtual void OnBeginPlay() {}

private:
	
	DeltaHandle NewHandle() const;
	bool InitializeNewObject(std::shared_ptr<DeltaObject> obj);

	void AddChildObject(std::shared_ptr<DeltaObject> _OwnedObject);
	void RemoveChildObject(std::shared_ptr<DeltaObject> obj);

protected:
	std::shared_ptr<class DeltaEngine> Engine;

private:
	std::string Name = "";
	
	const DeltaHandle Handle = 0;

	const std::weak_ptr<DeltaObject> Owner;
	std::list<std::weak_ptr<DeltaObject>> OwnedObjects;
};