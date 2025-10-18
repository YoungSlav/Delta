#include "stdafx.h"
#include "Renderer.h"
#include "Engine.h"
#include "Scene.h"
#include "StaticMeshComponent.h"
#include "StaticMesh.h"
#include "Pipeline.h"
#include "Camera.h"
#include "Window.h"
#include "Transform.h"
#include "VulkanCore.h"
#include "Material.h"
#include "AssetManager.h"
#include <unordered_map>

using namespace Delta;

bool Renderer::initialize_Internal()
{

	std::shared_ptr<VulkanCore> vk = engine->getVulkanCore();
	VkDevice device = vk->getDevice();

	createCameraUniformBuffer();
	vk->OnSwapchainRecreated.AddSP(Self<Renderer>(), &Renderer::onSwapchainRecreated);
	createGBufferResources();
	createDepthResources();
	createShadowResources();

	// Create global descriptor set layout (set=0) and per-frame descriptor sets for camera UBO
	{
		VkDescriptorSetLayoutBinding ubo{};
		ubo.binding = 0;
		ubo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		ubo.descriptorCount = 1;
		ubo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		ubo.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo li{};
		li.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		li.bindingCount = 1;
		li.pBindings = &ubo;
		if (vkCreateDescriptorSetLayout(device, &li, nullptr, &globalSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create global descriptor set layout");
		}

		globalSets.resize(MAX_FRAMES_IN_FLIGHT);
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, globalSetLayout);
		VkDescriptorSetAllocateInfo ai{};
		ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		ai.descriptorPool = vk->getDescriptorPool();
		ai.descriptorSetCount = (uint32)layouts.size();
		ai.pSetLayouts = layouts.data();
		if (vkAllocateDescriptorSets(device, &ai, globalSets.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate global descriptor sets");
		}

		for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			VkDescriptorBufferInfo bi{};
			bi.buffer = cameraUniformBuffers[i];
			bi.offset = 0;
			bi.range = sizeof(CameraInfo);

			VkWriteDescriptorSet write{};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = globalSets[i];
			write.dstBinding = 0;
			write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write.descriptorCount = 1;
			write.pBufferInfo = &bi;

			vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
		}
	}

	// Create geometry material descriptor set layout (set=1)
	{
		VkDescriptorSetLayoutBinding samp{};
		samp.binding = 0;
		samp.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samp.descriptorCount = 1;
		samp.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		samp.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo li{};
		li.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		li.bindingCount = 1;
		li.pBindings = &samp;
		if (vkCreateDescriptorSetLayout(device, &li, nullptr, &geomMaterialSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create geometry material descriptor set layout");
		}
	}

	// Create light pass descriptor set layout (set=2) and sampler
	{
		VkSamplerCreateInfo si{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
		si.magFilter = VK_FILTER_LINEAR; si.minFilter = VK_FILTER_LINEAR;
		si.addressModeU = si.addressModeV = si.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		vkCreateSampler(device, &si, nullptr, &gbufferSampler);

		std::array<VkDescriptorSetLayoutBinding,4> lbs{};
		for ( int32 i=0;i<4; ++i )
		{
			lbs[i].binding = i;
			lbs[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			lbs[i].descriptorCount = 1;
			lbs[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		}
		VkDescriptorSetLayoutCreateInfo lci{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
		lci.bindingCount = (uint32)lbs.size(); lci.pBindings = lbs.data();
		vkCreateDescriptorSetLayout(device, &lci, nullptr, &lightingSetLayout);

		lightingSets.resize(MAX_FRAMES_IN_FLIGHT);
		std::vector<VkDescriptorSetLayout> ls(MAX_FRAMES_IN_FLIGHT, lightingSetLayout);
		VkDescriptorSetAllocateInfo ai{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
		ai.descriptorPool = vk->getDescriptorPool();
		ai.descriptorSetCount = (uint32)ls.size(); ai.pSetLayouts = ls.data();
		vkAllocateDescriptorSets(device, &ai, lightingSets.data());

		for (uint32 f=0; f<MAX_FRAMES_IN_FLIGHT; ++f)
		{
			VkDescriptorImageInfo g0{ gbufferSampler, gAlbedo[f].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
			VkDescriptorImageInfo g1{ gbufferSampler, gNormal[f].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
			VkDescriptorImageInfo gd{ depthSampler,   depthRT[f].view,  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
			VkDescriptorImageInfo gs{ shadowSampler,  shadowMapRT[f].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

			std::array<VkWriteDescriptorSet,4> ws{};
			for (int i = 0; i < 4; ++i)
			{
				ws[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				ws[i].dstSet = lightingSets[f];
				ws[i].dstBinding = i;
				ws[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				ws[i].descriptorCount = 1;
			}
			ws[0].pImageInfo = &g0;
			ws[1].pImageInfo = &g1;
			ws[2].pImageInfo = &gd;
			ws[3].pImageInfo = &gs;


			vkUpdateDescriptorSets(device, (uint32)ws.size(), ws.data(), 0, nullptr);
		}
	}
	{
		Pipeline::Config cfg = Pipeline::MakeGeometryGBufferConfig(gAlbedoFormat, gNormalFormat, depthFormat);
		cfg.setLayouts = { globalSetLayout, geomMaterialSetLayout };
		gbufferPipeline = engine->getAssetManager()->findOrLoad<Pipeline>("gbufferPipeline", "Shaders/deffered_gbuffer", cfg);
	}
	{
		Pipeline::Config lcfg = Pipeline::MakeLightningConfig(vk);
		lcfg.setLayouts = { globalSetLayout, lightingSetLayout };
		lightingPipeline = engine->getAssetManager()->findOrLoad<Pipeline>("lightingPipeline", "Shaders/deffered_light", lcfg);
	}
	{
		Pipeline::Config scfg = Pipeline::MakeShadowConfig(vk);
		// Reuse global set 0; material layout can still be present but unused
		scfg.setLayouts = { globalSetLayout, geomMaterialSetLayout };
		shadowPipeline = engine->getAssetManager()->findOrLoad<Pipeline>("shadowPipeline", "Shaders/shadow", scfg);
	}

	return Object::initialize_Internal();
}

void Renderer::createCameraUniformBuffer()
{
	VkDeviceSize bufferSize = sizeof(CameraInfo);

	cameraUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	cameraUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
	cameraUniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		engine->getVulkanCore()->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, cameraUniformBuffers[i], cameraUniformBuffersMemory[i]);

		vkMapMemory(engine->getVulkanCore()->getDevice(), cameraUniformBuffersMemory[i], 0, bufferSize, 0, &cameraUniformBuffersMapped[i]);
	}
}

void Renderer::onDestroy()
{
	cleanup();
	Object::onDestroy();
}

void Renderer::cleanup()
{
	engine->getVulkanCore()->OnSwapchainRecreated.RemoveObject(this);
	destroyGBufferResources();
	destroyDepthResources();
	destroyShadowResources();

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroyBuffer(engine->getVulkanCore()->getDevice(), cameraUniformBuffers[i], nullptr);
		vkFreeMemory(engine->getVulkanCore()->getDevice(), cameraUniformBuffersMemory[i], nullptr);
	}
	if (globalSetLayout)
	{
		vkDestroyDescriptorSetLayout(engine->getVulkanCore()->getDevice(), globalSetLayout, nullptr);
		globalSetLayout = VK_NULL_HANDLE;
	}
	if (geomMaterialSetLayout)
	{
		vkDestroyDescriptorSetLayout(engine->getVulkanCore()->getDevice(), geomMaterialSetLayout, nullptr);
		geomMaterialSetLayout = VK_NULL_HANDLE;
	}
	if (lightingSetLayout)
	{
		vkDestroyDescriptorSetLayout(engine->getVulkanCore()->getDevice(), lightingSetLayout, nullptr);
		lightingSetLayout = VK_NULL_HANDLE;
	}
	if (gbufferSampler)
	{
		vkDestroySampler(engine->getVulkanCore()->getDevice(), gbufferSampler, nullptr);
		gbufferSampler = VK_NULL_HANDLE;
	}
}

void Renderer::createGBufferResources()
{
	std::shared_ptr<VulkanCore> vk = engine->getVulkanCore();
	const uint32 w = vk->getSwapchainExtent().width;
	const uint32 h = vk->getSwapchainExtent().height;

	for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		// Albedo + Metallic
		vk->createImage(
			w, h, 1,
			gAlbedoFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			gAlbedo[i].image, gAlbedo[i].memory);
		gAlbedo[i].view = vk->createImageView(gAlbedo[i].image, gAlbedoFormat, VK_IMAGE_ASPECT_COLOR_BIT);

		// Normal + Roughness
		vk->createImage(
			w, h, 1,
			gNormalFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			gNormal[i].image, gNormal[i].memory);
		gNormal[i].view = vk->createImageView(gNormal[i].image, gNormalFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	// Initialize layouts
	vk->singleTimeCommand(EQueueType::GRAPHICS, [&](VkCommandBuffer cmd)
	{
		for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			vk->transitionImageLayout(cmd, gAlbedo[i].image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			vk->transitionImageLayout(cmd, gNormal[i].image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		}
	});
}

void Renderer::destroyGBufferResources()
{
	std::shared_ptr<VulkanCore> vk = engine->getVulkanCore();
	for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		if (gAlbedo[i].view) vkDestroyImageView(vk->getDevice(), gAlbedo[i].view, nullptr);
		vk->destroyImage(gAlbedo[i].image, gAlbedo[i].memory);
		gAlbedo[i] = {};

		if (gNormal[i].view) vkDestroyImageView(vk->getDevice(), gNormal[i].view, nullptr);
		vk->destroyImage(gNormal[i].image, gNormal[i].memory);
		gNormal[i] = {};
	}
}

void Renderer::createDepthResources()
{
	std::shared_ptr<VulkanCore> vk = engine->getVulkanCore();
	depthFormat = vk->getDepthFormatPublic();
	const uint32 w = vk->getSwapchainExtent().width;
	const uint32 h = vk->getSwapchainExtent().height;

	for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vk->createImage(
			w, h, 1,
			depthFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			depthRT[i].image, depthRT[i].memory);
		depthRT[i].view = vk->createImageView(depthRT[i].image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
	}

	vk->singleTimeCommand(EQueueType::GRAPHICS, [&](VkCommandBuffer cb)
	{
		for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			vk->transitionImageLayout(cb, depthRT[i].image, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		}
	});

	VkSamplerCreateInfo si{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
	si.magFilter = VK_FILTER_NEAREST;
	si.minFilter = VK_FILTER_NEAREST;
	si.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	si.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	si.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	si.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	si.mipLodBias = 0.0f;
	si.anisotropyEnable = VK_FALSE;
	si.compareEnable = VK_FALSE;
	si.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	si.minLod = 0.0f;
	si.maxLod = 0.0f;
	si.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	si.unnormalizedCoordinates = VK_FALSE;

	if (vkCreateSampler(vk->getDevice(), &si, nullptr, &depthSampler) != VK_SUCCESS)
		throw std::runtime_error("failed to create depth sampler");
}

void Renderer::destroyDepthResources()
{
	std::shared_ptr<VulkanCore> vk = engine->getVulkanCore();
	for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		if (depthRT[i].view) vkDestroyImageView(vk->getDevice(), depthRT[i].view, nullptr);
		vk->destroyImage(depthRT[i].image, depthRT[i].memory);
		depthRT[i] = {};
	}

	if (shadowSampler) { vkDestroySampler(vk->getDevice(), shadowSampler, nullptr); shadowSampler = VK_NULL_HANDLE; }

}

void Renderer::createShadowResources()
{
	std::shared_ptr<VulkanCore> vk = engine->getVulkanCore();
	depthFormat = vk->getDepthFormatPublic();
	const uint32 w = vk->getSwapchainExtent().width;
	const uint32 h = vk->getSwapchainExtent().height;

	for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vk->createImage(
			w, h, 1,
			depthFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			shadowMapRT[i].image, shadowMapRT[i].memory);
		shadowMapRT[i].view = vk->createImageView(shadowMapRT[i].image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
	}

	vk->singleTimeCommand(EQueueType::GRAPHICS, [&](VkCommandBuffer cb)
	{
		for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			vk->transitionImageLayout(cb, shadowMapRT[i].image, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		}
	});

	VkSamplerCreateInfo si{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
	si.magFilter = VK_FILTER_LINEAR;
	si.minFilter = VK_FILTER_LINEAR;
	si.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	si.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	si.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	si.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	si.mipLodBias = 0.0f;
	si.anisotropyEnable = VK_FALSE;
	si.compareEnable = VK_FALSE;
	si.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	si.minLod = 0.0f;
	si.maxLod = 0.0f;
	si.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	si.unnormalizedCoordinates = VK_FALSE;

	if (vkCreateSampler(vk->getDevice(), &si, nullptr, &shadowSampler) != VK_SUCCESS)
		throw std::runtime_error("failed to create shadow sampler");
}

void Renderer::destroyShadowResources()
{
	std::shared_ptr<VulkanCore> vk = engine->getVulkanCore();
	for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		if (shadowMapRT[i].view) vkDestroyImageView(vk->getDevice(), shadowMapRT[i].view, nullptr);
		vk->destroyImage(shadowMapRT[i].image, shadowMapRT[i].memory);
		shadowMapRT[i] = {};
	}

    if (depthSampler)  { vkDestroySampler(vk->getDevice(), depthSampler,  nullptr); depthSampler  = VK_NULL_HANDLE; }

}

void Renderer::onSwapchainRecreated()
{
    // Rebuild render targets to match new extent/format
    destroyGBufferResources();
    createGBufferResources();
    destroyDepthResources();
    createDepthResources();

	// After recreating GBuffer, update lighting descriptor sets with new image views
	{
		std::shared_ptr<VulkanCore> vk = engine->getVulkanCore();
		VkDevice device = vk->getDevice();
		for (uint32 f = 0; f < MAX_FRAMES_IN_FLIGHT; ++f)
		{
			VkDescriptorImageInfo g0{ gbufferSampler, gAlbedo[f].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
			VkDescriptorImageInfo g1{ gbufferSampler, gNormal[f].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
			VkDescriptorImageInfo gd{ depthSampler,   depthRT[f].view,  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
			VkDescriptorImageInfo gs{ shadowSampler,  shadowMapRT[f].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

			std::array<VkWriteDescriptorSet,4> ws{};
			for (int i = 0; i < 4; ++i)
			{
				ws[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				ws[i].dstSet = lightingSets[f];
				ws[i].dstBinding = i;
				ws[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				ws[i].descriptorCount = 1;
			}
			ws[0].pImageInfo = &g0;
			ws[1].pImageInfo = &g1;
			ws[2].pImageInfo = &gd;
			ws[3].pImageInfo = &gs;

			vkUpdateDescriptorSets(device, (uint32)ws.size(), ws.data(), 0, nullptr);
		}
	}
}

VkDescriptorSet Renderer::getOrCreateMaterialSet(const std::shared_ptr<Material>& mat)
{
	const Material* key = mat.get();
	auto it = materialSetCache.find(key);
	if (it != materialSetCache.end())
		return it->second;

	// Allocate one descriptor set for this material
	VkDescriptorSet set = VK_NULL_HANDLE;
	VkDescriptorSetAllocateInfo ai{};
	ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	ai.descriptorPool = engine->getVulkanCore()->getDescriptorPool();
	ai.descriptorSetCount = 1;
	ai.pSetLayouts = &geomMaterialSetLayout;
	if (vkAllocateDescriptorSets(engine->getVulkanCore()->getDevice(), &ai, &set) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate material descriptor set");
	}

	VkDescriptorImageInfo ii{};
	ii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	ii.imageView = mat->getAlbedoImageView();
	ii.sampler = mat->getAlbedoSampler();

	VkWriteDescriptorSet w{};
	w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	w.dstSet = set;
	w.dstBinding = 0;
	w.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	w.descriptorCount = 1;
	w.pImageInfo = &ii;

	vkUpdateDescriptorSets(engine->getVulkanCore()->getDevice(), 1, &w, 0, nullptr);

	materialSetCache.emplace(key, set);
	return set;
}

void Renderer::updateCameraUniformBuffer(const CameraInfo& cameraInfo, uint32 currentImage)
{
	memcpy(cameraUniformBuffersMapped[currentImage], &cameraInfo, sizeof(cameraInfo));
}

void Renderer::drawFrame(const std::shared_ptr<class Scene> scene)
{
	engine->getVulkanCore()->drawFrame(
		[&](VkCommandBuffer cmd, uint32 currentFrame, uint32 imageIndex)
		{
			std::shared_ptr<VulkanCore> vk = engine->getVulkanCore();

			CameraInfo cameraInfo;
			scene->getCameraInfo(cameraInfo, engine->getWindow()->getViewportSize());
			updateCameraUniformBuffer(cameraInfo, currentFrame);


			// geometry
			VkRenderingAttachmentInfo colorInfos[2]{};
			colorInfos[0].imageView = gAlbedo[currentFrame].view;
			colorInfos[0].clearValue.color = {{0,0,0,1}};
			colorInfos[0].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			colorInfos[0].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorInfos[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorInfos[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			
			colorInfos[1].imageView = gNormal[currentFrame].view;
			colorInfos[1].clearValue.color = {{0,0,0,0}};
			colorInfos[1].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			colorInfos[1].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorInfos[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorInfos[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;

			VkRenderingAttachmentInfo depthInfo{};
			depthInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			depthInfo.imageView = depthRT[currentFrame].view;
			depthInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthInfo.clearValue.depthStencil = {1.0f, 0};

			VkRenderingInfo geomRI{};
			geomRI.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
			geomRI.renderArea = {{0,0}, vk->getSwapchainExtent()};
			geomRI.layerCount = 1;
			geomRI.colorAttachmentCount = 2;
			geomRI.pColorAttachments = colorInfos;
			geomRI.pDepthAttachment = &depthInfo;

			vkCmdBeginRendering(cmd, &geomRI);
			{
				VkViewport vp{0,0, (float)vk->getSwapchainExtent().width, (float)vk->getSwapchainExtent().height, 0.0f, 1.0f};
				vkCmdSetViewport(cmd, 0, 1, &vp);
				
				VkRect2D sc{{0,0}, vk->getSwapchainExtent()};
				vkCmdSetScissor(cmd, 0, 1, &sc);

				vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gbufferPipeline->getPipeline());

				VkDescriptorSet cameraDescriptorSet = globalSets[currentFrame];
				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gbufferPipeline->getPipelineLayout(), 0, 1, &cameraDescriptorSet, 0, nullptr);

				std::vector<std::shared_ptr<StaticMeshComponent>> renderGeometry;
				scene->getRenderGeometry(renderGeometry);
				for ( auto it : renderGeometry )
				{
					std::shared_ptr<StaticMesh> staticMesh = it->getMesh();
                	std::shared_ptr<Material> material = it->getMaterial();
					Transform transform = it->getTransform_World();
					glm::mat4 model = transform.getTransformMatrix();

					VkBuffer vertexBuffer, indexBuffer; uint32 indexCount;
					staticMesh->getBuffers(vertexBuffer, indexBuffer, indexCount);

					VkBuffer vertexBuffers[] = {vertexBuffer};
					VkDeviceSize offsets[] = {0};
					vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
					vkCmdBindIndexBuffer(cmd, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

					VkDescriptorSet set0 = globalSets[currentFrame];
					VkDescriptorSet set1 = getOrCreateMaterialSet(material);

					vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gbufferPipeline->getPipelineLayout(), 0, 1, &set0, 0, nullptr);
					vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gbufferPipeline->getPipelineLayout(), 1, 1, &set1, 0, nullptr);
					vkCmdPushConstants(cmd, gbufferPipeline->getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &model);

					vkCmdDrawIndexed(cmd, indexCount, 1, 0, 0, 0);
				}
			}
			vkCmdEndRendering(cmd);
		
			
			
			// light
			vk->transitionImageLayout(cmd, gAlbedo[currentFrame].image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			vk->transitionImageLayout(cmd, gNormal[currentFrame].image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			vk->transitionImageLayout(cmd, vk->getSwapchainImage(imageIndex), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

			VkRenderingAttachmentInfo outColor{};
			outColor.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			outColor.imageView = vk->getSwapchainImageView(imageIndex);
			outColor.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			outColor.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			outColor.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			outColor.clearValue.color = {{0,0,0,1}};

			VkRenderingInfo lightRI{VK_STRUCTURE_TYPE_RENDERING_INFO};
			lightRI.renderArea = {{0,0}, vk->getSwapchainExtent()};
			lightRI.layerCount = 1;
			lightRI.colorAttachmentCount = 1;
			lightRI.pColorAttachments = &outColor;

			vkCmdBeginRendering(cmd, &lightRI);
			{
				// viewport/scissor (reuse)
				VkViewport vp{0,0,(float)vk->getSwapchainExtent().width,(float)vk->getSwapchainExtent().height,0.0f,1.0f};
				vkCmdSetViewport(cmd, 0, 1, &vp);
				VkRect2D sc{{0,0}, vk->getSwapchainExtent()};
				vkCmdSetScissor(cmd, 0, 1, &sc);

				// bind and draw
				VkDescriptorSet set0 = globalSets[currentFrame];
				VkDescriptorSet set1 = lightingSets[currentFrame];

				vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, lightingPipeline->getPipeline());
				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, lightingPipeline->getPipelineLayout(), 0, 1, &set0, 0, nullptr);
				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, lightingPipeline->getPipelineLayout(), 1, 1, &set1, 0, nullptr);

				vkCmdDraw(cmd, 3, 1, 0, 0);
			}
			vkCmdEndRendering(cmd);

			// revert gbuffer back to color attachment optimal
			vk->transitionImageLayout(cmd, gAlbedo[currentFrame].image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			vk->transitionImageLayout(cmd, gNormal[currentFrame].image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

			// make swap chain image ready to present
			vk->transitionImageLayout( cmd, vk->getSwapchainImage(imageIndex), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		}
	);
}
