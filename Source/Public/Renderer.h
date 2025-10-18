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
	VkDescriptorSetLayout getGlobalSetLayout() const { return globalSetLayout; }

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

	void createShadowResources();
	void destroyShadowResources();

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
	VkFormat gAlbedoFormat = VK_FORMAT_R8G8B8A8_UNORM;		// Albedo + Metallic
	VkFormat gNormalFormat = VK_FORMAT_R16G16B16A16_SFLOAT;	// Normal + Roughness
	VkSampler gbufferSampler = VK_NULL_HANDLE;

	VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
	
	GBufferRT depthRT[MAX_FRAMES_IN_FLIGHT]{};
	VkSampler depthSampler;
	
	GBufferRT shadowMapRT[MAX_FRAMES_IN_FLIGHT]{};
	VkSampler shadowSampler;
	VkExtent2D shadowExtent = {2048,2048};
	
	// Descriptor layouts and per-frame global descriptor sets (camera UBO)
	VkDescriptorSetLayout globalSetLayout = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> globalSets;

	// Geometry material descriptor layout and cached sets per material
	VkDescriptorSetLayout geomMaterialSetLayout = VK_NULL_HANDLE;
	std::unordered_map<const class Material*, VkDescriptorSet> materialSetCache;
	VkDescriptorSet getOrCreateMaterialSet(const std::shared_ptr<class Material>& mat);

	VkDescriptorSetLayout lightingSetLayout = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> lightingSets; // size = MAX_FRAMES_IN_FLIGHT

	std::shared_ptr<class Pipeline> gbufferPipeline;
	std::shared_ptr<class Pipeline> lightingPipeline;
	std::shared_ptr<class Pipeline> shadowPipeline;

};

}

