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
	std::shared_ptr<class Renderer> getRenderer() const { return renderer; }

	void openScene(std::shared_ptr<class Scene> inScene);


	std::shared_ptr<class Scene> getScene() const { return scene; }

	inline bool isShutingDown() const { return bRequestExit; }
	void shutDown() { bRequestExit = true; }

protected:
	bool initialize_Internal() override;
	virtual void onDestroy() override;

private:
	void fireFreshBeginPlays();

private:

	std::shared_ptr<class Window> window;
	std::shared_ptr<class Input> input;
	std::shared_ptr<class VulkanCore> vulkanCore;
	std::shared_ptr<class AssetManager> assetManager;
	std::shared_ptr<class Renderer> renderer;


	std::shared_ptr<class Scene> scene;


	
	// object managing
	std::list< std::shared_ptr<Object> > objects;
	std::map<DeltaHandle, std::shared_ptr<Object>> handleToObject;

	// freshly created objects awaiting BeginPlay call
	std::list<std::shared_ptr<class Object>> freshObjects;

	DeltaHandle lastUsedHandle = 0;

	float gameTime = 0.0f;
	bool bRequestExit = false;

	float fpsLimit = 100000.0f;
};

}