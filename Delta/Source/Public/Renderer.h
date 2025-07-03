#pragma once
#include "stdafx.h"

#include "Object.h"




namespace Delta
{




class Renderer final : public Object
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
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
	Renderer(Args&&... args) :
		Object(std::forward<Args>(args)...)
	{}

	void DrawFrame(float DeltaTime);

	VkRenderPass GetRenderPass() { return RenderPass; }
	VkDevice GetDevice() { return Device; }

	std::shared_ptr<class Material> TempMaterialPtr;

protected:
	bool Initialize_Internal() override;
	virtual void OnDestroy() override;

private:

	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	void CreateInstance();
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void SetupDebugMessenger();
	void CreateSurface();
	void PickPhysicalDevice();
	void CreateLogicalDevice();
	void CreateSwapChain();
	void CreateImageViews();
	void CreateRenderPass();
	void CreateFramebuffers();
	void CreateCommandPool();
	void CreateCommandBuffer();
	void CreateSyncObjects();

	void OnWindowResize(const glm::ivec2& NewSize);

	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
	bool IsDeviceSuitable(VkPhysicalDevice device);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
	std::vector<const char*> GetRequiredExtensions();
	bool CheckValidationLayerSupport();
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

	void Cleanup();

private:

#ifdef NDEBUG
	const bool EnableValidationLayers = false;
#else
	const bool EnableValidationLayers = true;
#endif
	const std::vector<const char*> ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
	const std::vector<const char*> DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };


	VkInstance Instance;
	VkDebugUtilsMessengerEXT DebugMessenger;
	VkSurfaceKHR Surface;

	VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
	VkDevice Device;

	VkQueue GraphicsQueue;
	VkQueue PresentQueue;

	VkSwapchainKHR SwapChain;
	std::vector<VkImage> SwapChainImages;
	VkFormat SwapChainImageFormat;
	VkExtent2D SwapChainExtent;
	std::vector<VkImageView> SwapChainImageViews;
	std::vector<VkFramebuffer> SwapChainFramebuffers;

	VkRenderPass RenderPass;

	VkCommandPool CommandPool;
	VkCommandBuffer CommandBuffer;

	VkSemaphore ImageAvailableSemaphore;
	VkSemaphore RenderFinishedSemaphore;
	VkFence InFlightFence;





	
};

}
