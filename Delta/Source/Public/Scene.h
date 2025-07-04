#pragma once
#include "stdafx.h"

#include "Object.h"




namespace Delta
{


class Scene final : public Object
{
	friend class Engine;
public:
	template <typename... Args>
	Scene(Args&&... args) :
		Object(std::forward<Args>(args)...)
	{}

	template<typename Class>
	int32 getAllObjects(std::list<std::shared_ptr<Class>>& OutList) const
	{
		int32 count = 0;
		OutList.clear();
		for ( auto it : objects )
		{
			if ( it.expired() ) continue;
			if ( std::shared_ptr<Class> asDesired = std::dynamic_pointer_cast<Class>(it.lock()) )
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
			if ( it.expired() ) continue;
			if ( std::shared_ptr<Class> asDesired = std::dynamic_pointer_cast<Class>(it.lock()) )
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

	void getCameraInfo(struct CameraInfo& outCameraInfo, const glm::ivec2& ViewportSize) const;
	void getRenderGeometry(std::vector<std::shared_ptr<class StaticMeshComponent>>& outGeometry) const;

	void setCamera(std::shared_ptr<class Camera> newCamera) { camera = newCamera; }


protected:
	bool initialize_Internal() override;
	void onDestroy() override;


private:
	void update(float deltaTime);
	void registerObject(std::shared_ptr<Object> newObject);
	void removeObject(std::shared_ptr<Object> object);

private:

	std::weak_ptr<class Camera> camera;
	std::list<std::weak_ptr<class Object>> objects;
	std::list<std::weak_ptr<class Actor>> actors;
	std::list<std::weak_ptr<class ITickable>> tickableObjects;

	// renderable stuff
	std::list<std::weak_ptr<class StaticMeshComponent>> meshComponents;

};

}