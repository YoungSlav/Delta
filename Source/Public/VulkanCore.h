#pragma once
#include "stdafx.h"

#include "Object.h"





namespace Delta
{

#define MAX_FRAMES_IN_FLIGHT 2
#define MAX_MATERIAL_DESCRIPTOR_SETS 10


enum EQueueType
{
	GRAPHICS,
	TRANSFER,
	COMPUTE
};

class VulkanCore final : public Object
{
	struct QueueFamilyIndices
	{
		std::optional<uint32> graphicsFamily;
		std::optional<uint32> transferFamily;
		std::optional<uint32> presentFamily;

		bool isComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
		}
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};
public:
	template <typename... Args>
	VulkanCore(Args&&... args) :
		Object(std::forward<Args>(args)...)
	{}

	void singleTimeCommand(
		EQueueType queueType,
		const std::function<void(VkCommandBuffer)>& recordFunction);

	// recordFunction(VkCommandBuffer cmd, uint32 currentFrame, uint32 imageIndex)
	void drawFrame(const std::function<void(VkCommandBuffer, uint32, uint32)>& recordFunction);

	VkRenderPass getRenderPass() const { return renderPass; }
	VkDevice getDevice() const { return device; }
	VkExtent2D getSwapchainExtent() const { return swapChainExtent; }
	VkFormat getSwapchainFormat() const { return swapChainImageFormat; }
	VkImageView getSwapchainImageView(uint32 idx) const { return swapChainImageViews[idx]; }
	uint32 getSwapchainImageCount() const { return static_cast<uint32>(swapChainImages.size()); }
	VkImage getSwapchainImage(uint32 idx) const { return swapChainImages[idx]; }
	VkImageView getDepthImageView() const { return depthImageView; }
	VkImage getDepthImage() const { return depthImage; }

	VkPhysicalDeviceProperties  getPhysicalDeviceProperties() const { return physicalDeviceProperties; }
	VkDescriptorPool getDescriptorPool() const { return descriptorPool; }

	uint32 getMaxFramesInFlight() const { return MAX_FRAMES_IN_FLIGHT; }

	void createBuffer(
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	
	void createImage(
		uint32 width, uint32 height,
		uint32 mipLevels, VkFormat format,
		VkImageTiling tiling, VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkImage& image, VkDeviceMemory& imageMemory);

	void generateMipmaps(
		VkImage image, VkFormat imageFormat,
		int32 texWidth, int32 texHeight,
		uint32 mipLevels);

	void copyBufferToImage(
		VkBuffer buffer, VkImage image,
		uint32 width, uint32 height);

	VkImageView createImageView(
		VkImage image,
		VkFormat format,
		VkImageAspectFlags aspectFlags,
		uint32 mipLevels=1);

	void transitionImageLayout(
		VkImage image, VkFormat format,
		VkImageLayout oldLayout, VkImageLayout newLayout,
		uint32 mipLevels = 1);

	// Command-buffer variant for per-frame layout transitions (no submit/wait)
	void transitionImageLayoutCmd(
		VkCommandBuffer cmd,
		VkImage image, VkFormat format,
		VkImageLayout oldLayout, VkImageLayout newLayout,
		VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage,
		VkAccessFlags srcAccess, VkAccessFlags dstAccess,
		uint32 mipLevels = 1);

	// Expose depth format for dynamic rendering pipelines
	VkFormat getDepthFormatPublic();

protected:
	bool initialize_Internal() override;
	virtual void onDestroy() override;

private:

	uint32 findMemoryType(uint32 typeFilter, VkMemoryPropertyFlags properties);

	void recordCommandBuffer(
		VkCommandBuffer commandBuffer,
		uint32 imageIndex,
		const std::function<void(VkCommandBuffer, uint32, uint32)>& recordFunction);

	void createInstance();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void setupDebugMessenger();
	void createSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createSwapChain();
	void createImageViews();
	void createDepthResources();
	void createRenderPass();
	void createFramebuffers();
	void createDescriptorPool();
	void createRenderCommandPool();
	void createTransferCommandPool();
	void createRenderCommandBuffer();
	void createSyncObjects();

	void onWindowResize(const glm::ivec2& NewSize);
	void recreateSwapChain();

	VkFormat findDepthFormat();
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	bool hasStencilComponent(VkFormat format);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	std::vector<const char*> getRequiredExtensions();
	bool checkValidationLayerSupport();
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

	void cleanup();
	void cleanupSwapChain();

private:


#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif
	const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
	// Default device extensions; on macOS MoltenVK also requires VK_KHR_portability_subset
	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		"VK_KHR_dynamic_rendering",
		"VK_KHR_depth_stencil_resolve",
		"VK_KHR_synchronization2"
#if defined(__APPLE__)
		, "VK_KHR_portability_subset"
#endif
	};


	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties physicalDeviceProperties;
	VkDevice device;

	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkQueue transferQueue;

	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	VkRenderPass renderPass;

	VkCommandPool renderCommandPool;
	VkDescriptorPool descriptorPool;
	VkCommandPool transferCommandPool;

	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	int32 currentFrame = 0;
	bool bFamebufferResized = false;

};

}
