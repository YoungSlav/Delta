#include "DeltaEngine.h"
#include "IDeltaRenderable.h"

namespace GlobalEngine
{
	std::weak_ptr<DeltaEngine> _Engine;
	std::shared_ptr<DeltaEngine> Engine()
	{
		if( _Engine.expired() )
			return nullptr;
		return _Engine.lock();
	}
};


std::shared_ptr<DeltaEngine> DeltaEngine::GetEngine()
{
	return GlobalEngine::Engine();
}

bool DeltaEngine::Initialize_Internal()
{
	GlobalEngine::_Engine = Self<DeltaEngine>();
	
	Engine = Self<DeltaEngine>();

	LastUsedHandle = GetHandle();

	

	return true;
}


void DeltaEngine::GameLoop()
{
	//while( !bRequestExit && !glfwWindowShouldClose(Viewport->Window) )
	//{
	//	float newTime = (float)glfwGetTime();
	//	float DeltaTime = newTime - GameTime;
	//	GameTime = newTime;
	//	
	//	FireFreshBeginPlays();
	//
	//	Tick(DeltaTime);
	//	
	//
	//	while ((glfwGetTime()  - newTime) < (1.0 / FPSLimit)) { }
	//}
	//
	//glfwTerminate();
}

void DeltaEngine::FireFreshBeginPlays()
{
	for ( auto it : FreshObjects )
	{
		it->OnBeginPlay();
	}
	FreshObjects.clear();
}

void DeltaEngine::StopGame()
{
	while( Objects.size() > 0 )
	{
		DestroyObject(*Objects.cbegin());
	}

	//glfwSetWindowShouldClose(Viewport->Window, true);
}

void DeltaEngine::Tick(float DeltaTime)
{
	for ( auto it : TickableObjects )
		it->Update(DeltaTime);
}


void DeltaEngine::RegisterObject(std::shared_ptr<DeltaObject> newObject)
{
	Objects.push_back(newObject);
	
	FreshObjects.push_back(newObject);

	HandleToObject.insert({newObject->GetHandle(), newObject});
	
	if ( std::shared_ptr<IDeltaTickable> asTickable = std::dynamic_pointer_cast<IDeltaTickable>(newObject) )
	{
		TickableObjects.push_back(asTickable);
	}
}

std::shared_ptr<DeltaObject> DeltaEngine::FindObjectByHandle(const DeltaHandle& Handle)
{
	auto it = HandleToObject.find(Handle);
	if ( it != HandleToObject.end() )
		return it->second;
	return nullptr;
}

void DeltaEngine::DestroyObject(std::shared_ptr<DeltaObject> Object)
{
	HandleToObject.erase(Object->GetHandle());

	if ( std::shared_ptr<DeltaActor> asActor = std::dynamic_pointer_cast<DeltaActor>(Object) )
	{
		Actors.erase(std::remove(Actors.begin(), Actors.end(), asActor), Actors.end());
	}

	if ( std::shared_ptr<IDeltaTickable> asTiackble = std::dynamic_pointer_cast<IDeltaTickable>(Object) )
		TickableObjects.erase(std::remove(TickableObjects.begin(), TickableObjects.end(), asTiackble), TickableObjects.end());

	Objects.erase(std::remove(Objects.begin(), Objects.end(), Object), Objects.end());
}

void DeltaEngine::OnDestroy()
{
	DeltaObject::OnDestroy();
}