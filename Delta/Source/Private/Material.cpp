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

	std::vector<VkDescriptorSetLayout> layouts(1, pipeline->getMaterialDescriptorSetLayout());

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = engine->getVulkanCore()->getDescriptorPool();
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts.data();

	if ( vkAllocateDescriptorSets(engine->getVulkanCore()->getDevice(), &allocInfo, &materialDescriptorSet) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate material descriptor sets!");
	}
	
	VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = texture->getTextureImageView();
    imageInfo.sampler = texture->getTextureSampler();

	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = materialDescriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(engine->getVulkanCore()->getDevice(), 1, &descriptorWrite, 0, nullptr);

	return true;
}

void Material::onDestroy()
{
	Object::onDestroy();

	vkFreeDescriptorSets(engine->getVulkanCore()->getDevice(), engine->getVulkanCore()->getDescriptorPool(), 1, &materialDescriptorSet);
}