#include "stdafx.h"
#include "Texture.h"
#include "TextureData.h"
#include "Engine.h"
#include "VulkanCore.h"
#include "AssetManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

using namespace Delta;

EAssetLoadingState Texture::load_Internal()
{
	LOG_INDENT;

	TextureData textureData;

	const std::string fullPath = engine->getAssetManager()->findAsset(texturePath);

	textureData.pixels = stbi_load(fullPath.c_str(), &textureData.width, &textureData.height, &textureData.channels, STBI_rgb_alpha);
	VkDeviceSize imageSize = textureData.width * textureData.height * 4;

	if (!textureData.pixels)
	{
		LOG(Error, "Failed to load texture '{}' with {}", getName(), stbi_failure_reason());
		throw std::runtime_error("Failed to load texture!");
	}

	LOG(Log, "Loaded texture '{}', width: {}, height: {}, channels: {}", getName(), textureData.width, textureData.height, textureData.channels);

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	engine->getVulkanCore()->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* bufferData;
	vkMapMemory(engine->getVulkanCore()->getDevice(), stagingBufferMemory, 0, imageSize, 0, &bufferData);
		memcpy(bufferData, textureData.pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(engine->getVulkanCore()->getDevice(), stagingBufferMemory);

	stbi_image_free(textureData.pixels);

	engine->getVulkanCore()->createImage(textureData.width, textureData.height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	engine->getVulkanCore()->copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(textureData.width), static_cast<uint32_t>(textureData.height));
	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(engine->getVulkanCore()->getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(engine->getVulkanCore()->getDevice(), stagingBufferMemory, nullptr);

	textureImageView = engine->getVulkanCore()->createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);

	createTextureSampler();
	
	return EAssetLoadingState::LOADED;
}

void Texture::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	engine->getVulkanCore()->singleTimeCommand(EQueueType::GRAPHICS,
		[&](VkCommandBuffer commandBuffer)
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;

			if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
			{
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else
			{
				throw std::invalid_argument("unsupported layout transition!");
			}

			vkCmdPipelineBarrier( commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier );
		});
}

void Texture::createTextureSampler()
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = engine->getVulkanCore()->getPhysicalDeviceProperties().limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	if (vkCreateSampler(engine->getVulkanCore()->getDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture sampler!");
	}

}

void Texture::cleanup_Internal()
{
	vkDestroySampler(engine->getVulkanCore()->getDevice(), textureSampler, nullptr);
	vkDestroyImageView(engine->getVulkanCore()->getDevice(), textureImageView, nullptr);
	vkDestroyImage(engine->getVulkanCore()->getDevice(), textureImage, nullptr);
    vkFreeMemory(engine->getVulkanCore()->getDevice(), textureImageMemory, nullptr);
}