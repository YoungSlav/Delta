#pragma once
#include "stdafx.h"

#include "Asset.h"


namespace Delta
{



class Pipeline : public Asset
{
public:
	template <typename... Args>
	Pipeline(const std::string& inShaderName, Args&&... args) :
		Asset(std::forward<Args>(args)...),
		shaderName(inShaderName)
	{}

	VkPipeline getPipeline() const { return pipeline; }
	VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
	VkDescriptorSet getGlobalDescriptorSet(uint32 frame) const { return globalDescriptorSets[frame]; }
	VkDescriptorSetLayout getMaterialDescriptorSetLayout() const { return materialDescriptorSetLayout; };

protected:

	virtual EAssetLoadingState load_Internal() override;
	virtual void cleanup_Internal() override;

	void createDescriptorSetLayout();
	void createDescriptorSets();
	void createGraphicsPipeline();
	VkShaderModule createShaderModule(const std::vector<char>& code);
	static std::vector<char> readFile(const std::string& filename);

private:
	VkDescriptorSetLayout globalDescriptorSetLayout;
	std::vector<VkDescriptorSet> globalDescriptorSets;

	VkDescriptorSetLayout materialDescriptorSetLayout;

	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;

	const std::string shaderName = "";
};

}