#pragma once
#include "stdafx.h"
#include "SharedFromThis.h"

#include "Object.h"
#include "Actor.h"

#define TEST_MEMORY_LEAKS 0

namespace Delta
{

class Engine final : public Object
{
public:

	Engine(const std::string& _Name) :
		Object(0, _Name, nullptr, nullptr)
	{}

	void GameLoop();
	void StopGame();
		
	void RegisterObject(std::shared_ptr<Object> Object);
	std::shared_ptr<Object> FindObjectByHandle(const DeltaHandle& Handle);
	DeltaHandle GenerateNewHandle() { return ++LastUsedHandle; }
	void DestroyObject(std::shared_ptr<Object> Object);

	void LimitFPS(uint32 MaxFPS) { FPSLimit = MaxFPS > 0 ? static_cast<float>(MaxFPS) : 100000.0f; }

	static std::shared_ptr<Engine> GetEngine();

	std::shared_ptr<class Window> GetWindow() const { return WindowPtr; }
	std::shared_ptr<class Input> GetInput() const { return InputPtr; }
	std::shared_ptr<class Renderer> GetRenderer() const { return RendererPtr; }
	std::shared_ptr<class AssetManager> GetAssetManager() const { return AssetManagerPtr; }

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

private:
	void FireFreshBeginPlays();

	void Tick(float DeltaTime);
	
private:

	std::shared_ptr<class Window> WindowPtr;
	std::shared_ptr<class Input> InputPtr;
	std::shared_ptr<class Renderer> RendererPtr;
	std::shared_ptr<class AssetManager> AssetManagerPtr;

	// freshly created objects awaiting BeginPlay call
	std::list< std::shared_ptr<Object> > FreshObjects;

	// object managing
	std::list< std::shared_ptr<Object> > Objects;
	std::list< std::shared_ptr<class Actor> > Actors;
	std::map<DeltaHandle, std::shared_ptr<Object>> HandleToObject;
	std::list< std::shared_ptr<ITickable> > TickableObjects;

	DeltaHandle LastUsedHandle = 0;

	float GameTime = 0.0f;
	bool bRequestExit = false;

	float FPSLimit = 100000.0f;
};

}