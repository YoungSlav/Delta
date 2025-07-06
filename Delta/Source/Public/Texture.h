#pragma once

#include "stdafx.h"
#include "Asset.h"


namespace Delta
{

class Texture : public Asset
{
public:
	template <typename... Args>
	Texture(const std::string& inTexturePath, Args&&... args) :
		Asset(std::forward<Args>(args)...),
		texturePath(inTexturePath)
	{}

protected:
	virtual EAssetLoadingState load_Internal() override;
	virtual void cleanup_Internal() override;

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

private:
	const std::string texturePath = "";

	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
};

}