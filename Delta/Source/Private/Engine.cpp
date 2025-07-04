#include "Engine.h"
#include "IRenderable.h"
#include "Window.h"
#include "Input.h"
#include "VulkanCore.h"
#include "AssetManager.h"
#include "Scene.h"
#include "Renderer.h"

using namespace Delta;

namespace GlobalEngine
{
	std::weak_ptr<Engine> _Engine;
	std::shared_ptr<Engine> Engine()
	{
		if( _Engine.expired() )
			return nullptr;
		return _Engine.lock();
	}
};


std::shared_ptr<Engine> Engine::getEngine()
{
	return GlobalEngine::Engine();
}

bool Engine::initialize_Internal()
{
	GlobalEngine::_Engine = Self<Engine>();
	
	engine = Self<Engine>();

	lastUsedHandle = getHandle();

	LOG(Log, "Delta engine instance created!");

	assetManager = spawn<AssetManager>("AssetManager");
	window = spawn<Window>("Window");
	vulkanCore = spawn<VulkanCore>("Renderer");
	input = spawn<Input>("Input");

	renderer = spawn<Renderer>("Renderer");

	return true;
}


void Engine::gameLoop()
{
	LOG(Log, "MAIN LOOP STARTED");
	while( !bRequestExit && !glfwWindowShouldClose(window->getWindow()) )
	{
		float newTime = (float)glfwGetTime();
		float deltaTime = newTime - gameTime;
		gameTime = newTime;
		
		
		fireFreshBeginPlays();

		if ( scene )
		{
			scene->update(deltaTime);

			renderer->drawFrame(scene);
		}
		else
		{
			LOG(Error, "No scene is set to render!");
		}

		glfwPollEvents();

		if ( input )
			input->processInput(deltaTime);
	
		while ((glfwGetTime()  - newTime) < (1.0 / fpsLimit)) { }
	}

	LOG(Log, "Exit main loop");
	vkDeviceWaitIdle(vulkanCore->getDevice());
	this->destroy();
}

void Engine::openScene(std::shared_ptr<Scene> inScene)
{
	scene = inScene;
}

void Engine::fireFreshBeginPlays()
{
	for ( auto it : freshObjects )
	{
		if ( it.expired() ) continue;
		it.lock()->onBeginPlay();
	}
	freshObjects.clear();
}

std::shared_ptr<Object> Engine::findObjectByHandle(const DeltaHandle& Handle)
{
	auto it = handleToObject.find(Handle);
	if ( it != handleToObject.end() )
	{
		return it->second.expired() ? nullptr : it->second.lock();
	}
	return nullptr;
}

void Engine::registerObject(std::shared_ptr<Object> newObject)
{
	objects.push_back(newObject);
	
	freshObjects.push_back(newObject);

	handleToObject.insert({newObject->getHandle(), newObject});
	
	if ( scene )
		scene->registerObject(newObject);
}

void Engine::destroyObject(std::shared_ptr<Object> object)
{
	if ( scene )
		scene->removeObject(object);

	auto sameObject = [](auto& ptr, auto& other) {
		return !ptr.owner_before(other) && !other.owner_before(ptr);
	};

	objects.remove_if([&](const std::weak_ptr<Object>& weak) {
		return sameObject(weak, object);
	});

	for (auto it = handleToObject.begin(); it != handleToObject.end(); )
    {
        std::weak_ptr<Object>& weak = it->second;
        if (sameObject(weak, object))
        {
            it = handleToObject.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
