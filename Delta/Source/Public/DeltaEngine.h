#pragma once
#include "stdafx.h"
#include "SharedFromThis.h"

#include "DeltaObject.h"
#include "DeltaActor.h"

#define TEST_MEMORY_LEAKS 0


class DeltaInput;
class DeltaEngine : public DeltaObject
{
public:

	DeltaEngine(const std::string& _Name) :
		DeltaObject(0, _Name, nullptr, nullptr)
	{}

	void GameLoop();
	void StopGame();
		
	void RegisterObject(std::shared_ptr<DeltaObject> Object);
	std::shared_ptr<DeltaObject> FindObjectByHandle(const DeltaHandle& Handle);
	DeltaHandle GenerateNewHandle() { return ++LastUsedHandle; }
	void DestroyObject(std::shared_ptr<DeltaObject> Object);

	void LimitFPS(uint32 MaxFPS) { FPSLimit = MaxFPS > 0 ? static_cast<float>(MaxFPS) : 100000.0f; }

	static std::shared_ptr<DeltaEngine> GetEngine();

	template<typename Class>
	int32 GetAllObjects(std::list<std::shared_ptr<Class>>& OutList) const
	{
		int32 count = 0;
		OutList.clear();
		for ( auto it : Objects )
		{
			if ( std::shared_ptr<Class> asDesired = std::dynamic_pointer_cast<Class>(it) )
			{
				OutList.push_back(asDesired);
				++count;
			}
		}

		return count;
	}

	template<typename Class>
	int32 ActorsInRadius(const glm::vec3& AtLocation, float Radius, std::list<std::shared_ptr<Class>>& OutList) const
	{
		int32 count = 0;
		float Radius2 = Radius * Radius;
		OutList.clear();
		for ( auto it : Actors )
		{
			if ( std::shared_ptr<Class> asDesired = std::dynamic_pointer_cast<Class>(it) )
			{
				if ( glm::distance2(asDesired->GetLocation() - AtLocation) <= Radius2 )
				{
					OutList.push_back(asDesired);
					++count;
				}
			}
		}

		return count;
	}

	inline bool IsShutingDown() const { return bRequestExit; }
	void ShutDown() { bRequestExit = true; }

protected:
	bool Initialize_Internal() override;
	virtual void OnDestroy() override;

private:
	void FireFreshBeginPlays();

	void Tick(float DeltaTime);
	
private:

	// freshly created objects awaiting BeginPlay call
	std::list< std::shared_ptr<DeltaObject> > FreshObjects;

	// object managing
	std::list< std::shared_ptr<DeltaObject> > Objects;
	std::list< std::shared_ptr<class DeltaActor> > Actors;
	std::map<DeltaHandle, std::shared_ptr<DeltaObject>> HandleToObject;
	std::list< std::shared_ptr<IDeltaTickable> > TickableObjects;

	DeltaHandle LastUsedHandle = 0;

	float GameTime = 0.0f;
	bool bRequestExit = false;

	float FPSLimit = 100000.0f;
};