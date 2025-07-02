#pragma once
#include "stdafx.h"

#include "Object.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


namespace Delta
{



class Material : public Object
{
public:
	template <typename... Args>
	Material(Args&&... args) :
		Object(std::forward<Args>(args)...)
	{}

	VkPipeline GetGraphicsPipeline() const { return GraphicsPipeline; }

protected:

	virtual bool Initialize_Internal() override;
	virtual void OnDestroy() override;

	void CreateGraphicsPipeline();
	VkShaderModule CreateShaderModule(const std::vector<char>& code);
	static std::vector<char> ReadFile(const std::string& filename);

private:
	VkPipelineLayout PipelineLayout;
	VkPipeline GraphicsPipeline;
};

}