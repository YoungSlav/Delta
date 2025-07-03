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


std::shared_ptr<Engine> Engine::GetEngine()
{
	return GlobalEngine::Engine();
}

bool Engine::Initialize_Internal()
{
	GlobalEngine::_Engine = Self<Engine>();
	
	EnginePtr = Self<Engine>();

	LastUsedHandle = GetHandle();

	LOG(Log, "Delta engine instance created!");

	AssetManagerPtr = NewObject<AssetManager>("AssetManager");
	WindowPtr = NewObject<Window>("Window");
	VulkanCorePtr = NewObject<VulkanCore>("Renderer");
	InputPtr = NewObject<Input>("Input");


	return true;
}


void Engine::GameLoop()
{
	LOG(Log, "MAIN LOOP STARTED");
	while( !bRequestExit && !glfwWindowShouldClose(WindowPtr->GetWindow()) )
	{
		float newTime = (float)glfwGetTime();
		float DeltaTime = newTime - GameTime;
		GameTime = newTime;
		
		FireFreshBeginPlays();
	
		Tick(DeltaTime);
		
		//if ( GetCamera() )
		//	GetCamera()->UpdateCamera();
		//

		VulkanCorePtr->DrawFrame(DeltaTime);
		
		glfwPollEvents();

		if ( InputPtr )
			InputPtr->ProcessInput(DeltaTime);
	
		while ((glfwGetTime()  - newTime) < (1.0 / FPSLimit)) { }
	}

	LOG(Log, "Exit main loop");
	vkDeviceWaitIdle(VulkanCorePtr->GetDevice());
	this->Destroy();
}

void Engine::FireFreshBeginPlays()
{
	for ( auto it : FreshObjects )
	{
		it->OnBeginPlay();
	}
	FreshObjects.clear();
}

void Engine::Tick(float DeltaTime)
{
	for ( auto it : TickableObjects )
		it->Update(DeltaTime);
}


void Engine::RegisterObject(std::shared_ptr<Object> newObject)
{
	Objects.push_back(newObject);
	
	FreshObjects.push_back(newObject);

	HandleToObject.insert({newObject->GetHandle(), newObject});
	
	if ( std::shared_ptr<ITickable> asTickable = std::dynamic_pointer_cast<ITickable>(newObject) )
	{
		TickableObjects.push_back(asTickable);
	}
}

std::shared_ptr<Object> Engine::FindObjectByHandle(const DeltaHandle& Handle)
{
	auto it = HandleToObject.find(Handle);
	if ( it != HandleToObject.end() )
		return it->second;
	return nullptr;
}

void Engine::DestroyObject(std::shared_ptr<Object> Object)
{
	HandleToObject.erase(Object->GetHandle());

	if ( std::shared_ptr<Actor> asActor = std::dynamic_pointer_cast<Actor>(Object) )
	{
		Actors.erase(std::remove(Actors.begin(), Actors.end(), asActor), Actors.end());
	}

	if ( std::shared_ptr<ITickable> asTiackble = std::dynamic_pointer_cast<ITickable>(Object) )
		TickableObjects.erase(std::remove(TickableObjects.begin(), TickableObjects.end(), asTiackble), TickableObjects.end());

	Objects.erase(std::remove(Objects.begin(), Objects.end(), Object), Objects.end());
}
