#include "Material.h"
#include "Texture.h"
#include "VulkanCore.h"
#include "Engine.h"
#include "Pipeline.h"
#include "AssetManager.h"

using namespace Delta;

bool Material::initialize_Internal()
{
	Object::initialize_Internal();

	texture = engine->getAssetManager()->findOrLoad<Texture>(texturePath, texturePath);

	return true;
}

void Material::onDestroy()
{
	Object::onDestroy();
}

VkImageView Material::getAlbedoImageView() const { return texture ? texture->getTextureImageView() : VK_NULL_HANDLE; }
VkSampler Material::getAlbedoSampler() const { return texture ? texture->getTextureSampler() : VK_NULL_HANDLE; }
