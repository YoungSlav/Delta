#pragma once
#include "stdafx.h"

#include "Object.h"




namespace Delta
{


class Renderer : public Object
{
public:
	template <typename... Args>
	Renderer(Args&&... args) :
		Object(std::forward<Args>(args)...)
	{}

	void drawFrame(const std::shared_ptr<class Scene> scene);

	const std::vector<VkBuffer>& getCameraUniformBuffers() const { return cameraUniformBuffers; }

protected:
	virtual bool initialize_Internal() override;
	virtual void onDestroy() override;

	void cleanup();

	void createCameraUniformBuffer();
	void updateCameraUniformBuffer(const struct CameraInfo& cameraInfo, uint32 currentImage);

private:
	std::vector<VkBuffer> cameraUniformBuffers;
	std::vector<VkDeviceMemory> cameraUniformBuffersMemory;
	std::vector<void*> cameraUniformBuffersMapped;
};

}