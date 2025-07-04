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

	Engine(const std::string& inName) :
		Object(0, inName, nullptr, nullptr)
	{}

	void gameLoop();
		
	void registerObject(std::shared_ptr<Object> Object);
	std::shared_ptr<Object> findObjectByHandle(const DeltaHandle& Handle);
	DeltaHandle generateNewHandle() { return ++lastUsedHandle; }
	void destroyObject(std::shared_ptr<Object> Object);

	void limitFPS(uint32 MaxFPS) { fpsLimit = MaxFPS > 0 ? static_cast<float>(MaxFPS) : 100000.0f; }

	static std::shared_ptr<Engine> getEngine();

	std::shared_ptr<class Window> getWindow() const { return window; }
	std::shared_ptr<class Input> getInput() const { return input; }
	std::shared_ptr<class VulkanCore> getVulkanCore() const { return vulkanCore; }
	std::shared_ptr<class AssetManager> getAssetManager() const { return assetManager; }

	template<typename Class>
	int32 getAllObjects(std::list<std::shared_ptr<Class>>& OutList) const
	{
		int32 count = 0;
		OutList.clear();
		for ( auto it : objects )
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
	int32 getActorsInRadius(const glm::vec3& AtLocation, float Radius, std::list<std::shared_ptr<Class>>& OutList) const
	{
		int32 count = 0;
		float Radius2 = Radius * Radius;
		OutList.clear();
		for ( auto it : actors )
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

	inline bool isShutingDown() const { return bRequestExit; }
	void ihutDown() { bRequestExit = true; }

protected:
	bool initialize_Internal() override;

private:
	void fireFreshBeginPlays();

	void tick(float DeltaTime);
	
private:

	std::shared_ptr<class Window> window;
	std::shared_ptr<class Input> input;
	std::shared_ptr<class VulkanCore> vulkanCore;
	std::shared_ptr<class AssetManager> assetManager;

	// freshly created objects awaiting BeginPlay call
	std::list< std::shared_ptr<Object> > freshObjects;

	// object managing
	std::list< std::shared_ptr<Object> > objects;
	std::list< std::shared_ptr<class Actor> > actors;
	std::map<DeltaHandle, std::shared_ptr<Object>> handleToObject;
	std::list< std::shared_ptr<ITickable> > tickableObjects;

	DeltaHandle lastUsedHandle = 0;

	float gameTime = 0.0f;
	bool bRequestExit = false;

	float fpsLimit = 100000.0f;
};

}