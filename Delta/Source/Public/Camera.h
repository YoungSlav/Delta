#pragma once

#include "stdafx.h"
#include "ActorComponent.h"

namespace Delta
{

struct CameraInfo
{
	glm::mat4 View;
	glm::mat4 Proj;
	
	float FOV;
	float MinDist;
	float MaxDist;
};


class Camera : public ActorComponent
{
public:
	template <typename... Args>
	Camera(Args&&... args) :
		ActorComponent(std::forward<Args>(args)...)
	{}


	CameraInfo GetCameraInfo(const glm::ivec2& ViewportSize) const;

	// 3d camera
	void SetFOV(float InFOV) { FOV = InFOV; }
	void SetDrawingDistance(float InMin, float InMax) { MinDrawingDistance = InMin; MaxDrawingDistance = InMax; }


protected:

private:
	float FOV = 90.0f;
	float MinDrawingDistance = 0.1f;
	float MaxDrawingDistance = 1000.0f;
};
}