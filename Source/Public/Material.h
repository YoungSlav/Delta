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
	Material(const std::string& inTexturePath, Args&&... args) :
		Object(std::forward<Args>(args)...),
		texturePath(inTexturePath)
	{}

	// Expose texture resources; descriptor sets are created by Renderer
	VkImageView getAlbedoImageView() const;
	VkSampler getAlbedoSampler() const;

protected:
	virtual bool initialize_Internal() override;
	virtual void onDestroy() override;


private:
	const std::string texturePath;
	std::shared_ptr<class Texture> texture;
};

}
