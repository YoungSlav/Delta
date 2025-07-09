#pragma once
#include "stdafx.h"

#include "Object.h"




namespace Delta
{

#define MAX_FRAMES_IN_FLIGHT 2


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
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> transferFamily;
		std::optional<uint32_t> presentFamily;

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

	void singleTimeCommand(EQueueType queueType, const std::function<void(VkCommandBuffer)>& recordFunction);

	void drawFrame(const std::function<void(VkCommandBuffer, uint32_t)>& recordFunction);

	VkRenderPass getRenderPass() { return renderPass; }
	VkDevice getDevice() { return device; }
	VkPhysicalDeviceProperties  getPhysicalDeviceProperties() { return physicalDeviceProperties; }
	VkDescriptorPool getDescriptorPool() { return descriptorPool; }

	uint32 getMaxFramesInFlight() const { return MAX_FRAMES_IN_FLIGHT; }

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	VkImageView createImageView(VkImage image, VkFormat format);

protected:
	bool initialize_Internal() override;
	virtual void onDestroy() override;

private:

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32 imageIndex, const std::function<void(VkCommandBuffer, uint32)>& recordFunction);

	void createInstance();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void setupDebugMessenger();
	void createSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createSwapChain();
	void createImageViews();
	void createRenderPass();
	void createFramebuffers();
	void createDescriptorPool();
	void createRenderCommandPool();
	void createTransferCommandPool();
	void createRenderCommandBuffer();
	void createSyncObjects();

	void onWindowResize(const glm::ivec2& NewSize);
	void recreateSwapChain();

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
	const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };


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
