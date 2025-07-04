#include "Camera.h"
#include "DeltaMath.h"
#include "Engine.h"
#include "Window.h"

using namespace Delta;

CameraInfo Camera::fallbackCamera(const glm::ivec2& ViewportSize)
{
	CameraInfo outInfo;

	outInfo.fov = 90.0f;
	outInfo.minDist = 0.1f;
	outInfo.maxDist = 100.0f;


	const float AspectRatio = ViewportSize.y > 0.0f ? float(ViewportSize.x) / float(ViewportSize.y) : 0.0f;
	
	outInfo.view = glm::lookAt(glm::vec3(0.0f), Math::forwardV, Math::upV);
	outInfo.proj = glm::perspective(glm::radians(outInfo.fov*0.5f), AspectRatio, outInfo.minDist, outInfo.maxDist);
	outInfo.proj[1][1] *= -1;

	return outInfo;
}

CameraInfo Camera::getCameraInfo(const glm::ivec2& ViewportSize) const
{
	CameraInfo outInfo;

	outInfo.fov = fov;
	outInfo.minDist = minDrawingDistance;
	outInfo.maxDist = maxDrawingDistance;


	const float AspectRatio = ViewportSize.y > 0.0f ? float(ViewportSize.x) / float(ViewportSize.y) : 0.0f;
	
	outInfo.view = glm::lookAt(getLocation_World(), getLocation_World() + getForwardVector_World(), Math::upV);
	outInfo.proj = glm::perspective(glm::radians(fov*0.5f), AspectRatio, minDrawingDistance, maxDrawingDistance);
	outInfo.proj[1][1] *= -1;

	return outInfo;
}