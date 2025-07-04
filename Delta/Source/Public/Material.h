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

	VkPipeline getGraphicsPipeline() const { return graphicsPipeline; }

protected:

	virtual EAssetLoadingState load_Internal() override;
	virtual void cleanup_Internal() override;

	void createGraphicsPipeline();
	VkShaderModule createShaderModule(const std::vector<char>& code);
	static std::vector<char> readFile(const std::string& filename);

private:
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	const std::string shaderName = "";
};

}