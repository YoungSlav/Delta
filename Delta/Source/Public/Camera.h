#pragma once

#include "stdafx.h"
#include "ActorComponent.h"

namespace Delta
{

struct alignas(16) CameraInfo
{
	glm::mat4 view;
	glm::mat4 proj;
	
	float fov;
	float minDist;
	float maxDist;

	float padding;
};


class Camera : public ActorComponent
{
public:
	template <typename... Args>
	Camera(Args&&... args) :
		ActorComponent(std::forward<Args>(args)...)
	{}


	CameraInfo getCameraInfo(const glm::ivec2& ViewportSize) const;
	static CameraInfo fallbackCamera(const glm::ivec2& ViewportSize);

	// 3d camera
	void setFov(float InFOV) { fov = InFOV; }
	void setDrawingDistance(float InMin, float InMax) { minDrawingDistance = InMin; maxDrawingDistance = InMax; }


protected:

private:
	float fov = 90.0f;
	float minDrawingDistance = 0.1f;
	float maxDrawingDistance = 1000.0f;
};
}