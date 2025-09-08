#pragma once

#include "stdafx.h"
#include "Object.h"


namespace Delta
{

struct MaterialCreateInfo
{
	
};

class Material : public Object
{
public:
	template <typename... Args>
	Material(std::shared_ptr<class Pipeline> inPipeline, const std::string& inTexturePath, Args&&... args) :
		Object(std::forward<Args>(args)...),
		pipeline(inPipeline),
		texturePath(inTexturePath)
	{}

	VkDescriptorSet getMaterialDescriptorSet() const { return materialDescriptorSet; }

protected:
	virtual bool initialize_Internal() override;
	virtual void onDestroy() override;


private:
	const std::shared_ptr<class Pipeline> pipeline;

	const std::string texturePath;
	std::shared_ptr<class Texture> texture;

	VkDescriptorSet materialDescriptorSet;
};

}