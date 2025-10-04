#pragma once
#include "stdafx.h"

#include "Asset.h"


namespace Delta
{



class Pipeline : public Asset
{
public:
	struct Config
	{
		std::vector<VkFormat> colorAttachmentFormats; // dynamic rendering color formats
		bool enableDepthTest = true;
		bool enableDepthWrite = true;
		std::optional<VkFormat> depthAttachmentFormat; // std::nullopt for no depth
		VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
		VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	};

	template <typename... Args>
	Pipeline(const std::string& inShaderName, const Config& inConfig, Args&&... args) :
		Asset(std::forward<Args>(args)...),
		shaderName(inShaderName),
		config(inConfig)
	{}

	VkPipeline getPipeline() const { return pipeline; }
	VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
	VkDescriptorSet getGlobalDescriptorSet(uint32 frame) const { return globalDescriptorSets[frame]; }
	VkDescriptorSetLayout getMaterialDescriptorSetLayout() const { return materialDescriptorSetLayout; };

	// Helpers to build common configurations
	static Config MakeForwardConfig(std::shared_ptr<class VulkanCore> vk);
	static Config MakeGeometryGBufferConfig(VkFormat albedoFormat, VkFormat normalFormat, std::optional<VkFormat> depthFormat);

protected:

	virtual EAssetLoadingState load_Internal() override;
	virtual void cleanup_Internal() override;

	void createDescriptorSetLayout();
	void createDescriptorSets();
	void createGraphicsPipeline();
	VkShaderModule createShaderModule(const std::vector<char>& code);
	static std::vector<char> readFile(const std::string& filename);

private:
	Config config{};
	VkDescriptorSetLayout globalDescriptorSetLayout;
	std::vector<VkDescriptorSet> globalDescriptorSets;

	VkDescriptorSetLayout materialDescriptorSetLayout;

	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;

	const std::string shaderName = "";
};

}
