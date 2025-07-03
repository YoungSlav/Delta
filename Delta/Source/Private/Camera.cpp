#include "Camera.h"
#include "DeltaMath.h"
#include "Engine.h"
#include "Window.h"

using namespace Delta;

CameraInfo Camera::GetCameraInfo(const glm::ivec2& ViewportSize) const
{
	CameraInfo outInfo;

	outInfo.FOV = FOV;
	outInfo.MinDist = MinDrawingDistance;
	outInfo.MaxDist = MaxDrawingDistance;


	const float AspectRatio = ViewportSize.y > 0.0f ? float(ViewportSize.x) / float(ViewportSize.y) : 0.0f;
	
	outInfo.View = glm::lookAt(GetLocation_World(), GetLocation_World() + GetForwardVector_World(), Math::upV);
	outInfo.Proj = glm::perspective(glm::radians(FOV*0.5f), AspectRatio, MinDrawingDistance, MaxDrawingDistance);

	return outInfo;
}