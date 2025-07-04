#pragma once
#include "stdafx.h"

#include "Asset.h"


namespace Delta
{



class Material : public Asset
{
public:
	template <typename... Args>
	Material(const std::string& inShaderName, Args&&... args) :
		Asset(std::forward<Args>(args)...),
		shaderName(inShaderName)
	{}

	VkPipeline getPipeline() const { return pipeline; }
	VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
	VkDescriptorSet getDescriptorSet(uint32 frame) const { return descriptorSets[frame]; }

protected:

	virtual EAssetLoadingState load_Internal() override;
	virtual void cleanup_Internal() override;

	void createDescriptorSetLayout();
	void createDescriptorSets();
	void createGraphicsPipeline();
	VkShaderModule createShaderModule(const std::vector<char>& code);
	static std::vector<char> readFile(const std::string& filename);

private:
	VkDescriptorSetLayout descriptorSetLayout;
	std::vector<VkDescriptorSet> descriptorSets;

	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;

	const std::string shaderName = "";
};

}