#include "Engine.h"
#include "IRenderable.h"
#include "Window.h"
#include "Input.h"
#include "VulkanCore.h"
#include "AssetManager.h"

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


	return true;
}


void Engine::gameLoop()
{
	LOG(Log, "MAIN LOOP STARTED");
	while( !bRequestExit && !glfwWindowShouldClose(window->getWindow()) )
	{
		float newTime = (float)glfwGetTime();
		float DeltaTime = newTime - gameTime;
		gameTime = newTime;
		
		fireFreshBeginPlays();
	
		tick(DeltaTime);
		
		//if ( GetCamera() )
		//	GetCamera()->UpdateCamera();
		//

		vulkanCore->drawFrame(DeltaTime);
		
		glfwPollEvents();

		if ( input )
			input->processInput(DeltaTime);
	
		while ((glfwGetTime()  - newTime) < (1.0 / fpsLimit)) { }
	}

	LOG(Log, "Exit main loop");
	vkDeviceWaitIdle(vulkanCore->getDevice());
	this->destroy();
}

void Engine::fireFreshBeginPlays()
{
	for ( auto it : freshObjects )
	{
		it->onBeginPlay();
	}
	freshObjects.clear();
}

void Engine::tick(float DeltaTime)
{
	for ( auto it : tickableObjects )
		it->update(DeltaTime);
}


void Engine::registerObject(std::shared_ptr<Object> newObject)
{
	objects.push_back(newObject);
	
	freshObjects.push_back(newObject);

	handleToObject.insert({newObject->getHandle(), newObject});
	
	if ( std::shared_ptr<ITickable> asTickable = std::dynamic_pointer_cast<ITickable>(newObject) )
	{
		tickableObjects.push_back(asTickable);
	}
}

std::shared_ptr<Object> Engine::findObjectByHandle(const DeltaHandle& Handle)
{
	auto it = handleToObject.find(Handle);
	if ( it != handleToObject.end() )
		return it->second;
	return nullptr;
}

void Engine::destroyObject(std::shared_ptr<Object> Object)
{
	handleToObject.erase(Object->getHandle());

	if ( std::shared_ptr<Actor> asActor = std::dynamic_pointer_cast<Actor>(Object) )
	{
		actors.erase(std::remove(actors.begin(), actors.end(), asActor), actors.end());
	}

	if ( std::shared_ptr<ITickable> asTiackble = std::dynamic_pointer_cast<ITickable>(Object) )
		tickableObjects.erase(std::remove(tickableObjects.begin(), tickableObjects.end(), asTiackble), tickableObjects.end());

	objects.erase(std::remove(objects.begin(), objects.end(), Object), objects.end());
}
