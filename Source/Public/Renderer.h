#pragma once
#include "stdafx.h"
#include "Object.h"
#include "VulkanCore.h"



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

	// Swapchain-dependent GPU resources
	void createGBufferResources();
	void destroyGBufferResources();
	void onSwapchainRecreated();

	void createDepthResources();
	void destroyDepthResources();

private:
	std::vector<VkBuffer> cameraUniformBuffers;
	std::vector<VkDeviceMemory> cameraUniformBuffersMemory;
	std::vector<void*> cameraUniformBuffersMapped;

	struct GBufferRT
	{
		VkImage image = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
	};

	GBufferRT gAlbedo[MAX_FRAMES_IN_FLIGHT]{};
	GBufferRT gNormal[MAX_FRAMES_IN_FLIGHT]{};
	VkFormat gAlbedoFormat = VK_FORMAT_R8G8B8A8_UNORM;
	VkFormat gNormalFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

	GBufferRT depthRT[MAX_FRAMES_IN_FLIGHT]{};
	VkFormat depthFormat = VK_FORMAT_D32_SFLOAT; // will be set from VulkanCore::getDepthFormatPublic()
};

}
