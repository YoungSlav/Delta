#include "stdafx.h"
#include "Scene.h"
#include "Actor.h"
#include "StaticMeshComponent.h"
#include "Camera.h"

using namespace Delta;

bool Scene::initialize_Internal()
{
	return Object::initialize_Internal();
}

void Scene::onDestroy()
{
	Object::onDestroy();
}

void Scene::registerObject(std::shared_ptr<Object> newObject)
{
	objects.push_back(newObject);

	if ( std::shared_ptr<ITickable> asTickable = std::dynamic_pointer_cast<ITickable>(newObject) )
	{
		tickableObjects.push_back(asTickable);
	}
	if ( std::shared_ptr<StaticMeshComponent> asMesh = std::dynamic_pointer_cast<StaticMeshComponent>(newObject) )
	{
		meshComponents.push_back(asMesh);
	}
	if ( std::shared_ptr<Actor> asActor = std::dynamic_pointer_cast<Actor>(newObject) )
	{
		actors.push_back(asActor);
	}
}
void Scene::removeObject(std::shared_ptr<Object> object)
{
	auto sameObject = [](auto& ptr, auto& other) {
		return !ptr.owner_before(other) && !other.owner_before(ptr);
	};

	objects.remove_if([&](const std::weak_ptr<Object>& weak) {
		return sameObject(weak, object);
	});

	if (std::shared_ptr<ITickable> asTickable = std::dynamic_pointer_cast<ITickable>(object))
	{
		tickableObjects.remove_if([&](const std::weak_ptr<ITickable>& weak) {
			return sameObject(weak, asTickable);
		});
	}
	if (std::shared_ptr<Actor> asActor = std::dynamic_pointer_cast<Actor>(object))
	{
		actors.remove_if([&](const std::weak_ptr<Actor>& weak) {
			return sameObject(weak, asActor);
		});
	}

	if (std::shared_ptr<StaticMeshComponent> asMesh = std::dynamic_pointer_cast<StaticMeshComponent>(object))
	{
		meshComponents.remove_if([&](const std::weak_ptr<StaticMeshComponent>& weak) {
			return sameObject(weak, asMesh);
		});
	}
}

void Scene::update(float DeltaTime)
{
	for ( auto it : tickableObjects )
	{
		if ( it.expired() ) continue;
		it.lock()->update(DeltaTime);
	}
}

void Scene::getCameraInfo(CameraInfo& outCameraInfo, const glm::ivec2& ViewportSize) const
{
	if ( camera.expired() )
	{
		outCameraInfo = Camera::fallbackCamera(ViewportSize);
		LOG(Error, "No camera is set for the scene!");
	}

	outCameraInfo = camera.lock()->getCameraInfo(ViewportSize);
}

void Scene::getRenderGeometry(std::vector<std::shared_ptr<class StaticMeshComponent>>& outGeometry) const
{
	outGeometry.clear();
	outGeometry.reserve(meshComponents.size());
	for ( auto meshComponent : meshComponents )
	{
		if ( meshComponent.expired() ) continue;

		outGeometry.push_back(meshComponent.lock());
	}
}