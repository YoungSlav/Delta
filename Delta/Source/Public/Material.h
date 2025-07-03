#pragma once
#include "stdafx.h"

#include "Asset.h"


namespace Delta
{



class Material : public Asset
{
public:
	template <typename... Args>
	Material(const std::string& _ShaderName, Args&&... args) :
		Asset(std::forward<Args>(args)...),
		ShaderName(_ShaderName)
	{}

	VkPipeline GetGraphicsPipeline() const { return GraphicsPipeline; }

protected:

	virtual EAssetLoadingState Load_Internal() override;
	virtual void Cleanup_Internal() override;

	void CreateGraphicsPipeline();
	VkShaderModule CreateShaderModule(const std::vector<char>& code);
	static std::vector<char> ReadFile(const std::string& filename);

private:
	VkPipelineLayout PipelineLayout;
	VkPipeline GraphicsPipeline;

	const std::string ShaderName = "";
};

}