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
	createCameraUniformBuffer();
	engine->getVulkanCore()->OnSwapchainRecreated.AddSP(Self<Renderer>(), &Renderer::onSwapchainRecreated);
	createGBufferResources();
	createDepthResources();

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
		if (vkCreateDescriptorSetLayout(engine->getVulkanCore()->getDevice(), &li, nullptr, &globalSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create global descriptor set layout");
		}

		globalSets.resize(MAX_FRAMES_IN_FLIGHT);
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, globalSetLayout);
		VkDescriptorSetAllocateInfo ai{};
		ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		ai.descriptorPool = engine->getVulkanCore()->getDescriptorPool();
		ai.descriptorSetCount = (uint32)layouts.size();
		ai.pSetLayouts = layouts.data();
		if (vkAllocateDescriptorSets(engine->getVulkanCore()->getDevice(), &ai, globalSets.data()) != VK_SUCCESS)
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

			vkUpdateDescriptorSets(engine->getVulkanCore()->getDevice(), 1, &write, 0, nullptr);
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
		if (vkCreateDescriptorSetLayout(engine->getVulkanCore()->getDevice(), &li, nullptr, &geomMaterialSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create geometry material descriptor set layout");
		}
	}

    Pipeline::Config cfg = Pipeline::MakeForwardConfig(engine->getVulkanCore());
    cfg.setLayouts = { globalSetLayout, geomMaterialSetLayout };
    forwardPipelienTmp = engine->getAssetManager()->findOrLoad<Pipeline>("forwardPipelienTmp", "Shaders/triangle", cfg);
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
	vk->singleTimeCommand(EQueueType::GRAPHICS, [&](VkCommandBuffer cb)
	{
		for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			vk->transitionImageLayout(cb, gAlbedo[i].image, gAlbedoFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			vk->transitionImageLayout(cb, gNormal[i].image, gNormalFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		}
	});
}

void Renderer::destroyGBufferResources()
{
	std::shared_ptr<VulkanCore> vk = engine->getVulkanCore();
	for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		if (gAlbedo[i].view) vkDestroyImageView(vk->getDevice(), gAlbedo[i].view, nullptr);
		if (gAlbedo[i].image) vkDestroyImage(vk->getDevice(), gAlbedo[i].image, nullptr);
		if (gAlbedo[i].memory) vkFreeMemory(vk->getDevice(), gAlbedo[i].memory, nullptr);
		gAlbedo[i] = {};

		if (gNormal[i].view) vkDestroyImageView(vk->getDevice(), gNormal[i].view, nullptr);
		if (gNormal[i].image) vkDestroyImage(vk->getDevice(), gNormal[i].image, nullptr);
		if (gNormal[i].memory) vkFreeMemory(vk->getDevice(), gNormal[i].memory, nullptr);
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
			vk->transitionImageLayout(cb, depthRT[i].image, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		}
	});
}

void Renderer::destroyDepthResources()
{
	std::shared_ptr<VulkanCore> vk = engine->getVulkanCore();
	for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		if (depthRT[i].view) vkDestroyImageView(vk->getDevice(), depthRT[i].view, nullptr);
		if (depthRT[i].image) vkDestroyImage(vk->getDevice(), depthRT[i].image, nullptr);
		if (depthRT[i].memory) vkFreeMemory(vk->getDevice(), depthRT[i].memory, nullptr);
		depthRT[i] = {};
	}
}

void Renderer::onSwapchainRecreated()
{
	// Rebuild render targets to match new extent/format
	destroyGBufferResources();
	createGBufferResources();
	destroyDepthResources();
	createDepthResources();
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

			std::vector<std::shared_ptr<StaticMeshComponent>> renderGeometry;
			scene->getRenderGeometry(renderGeometry);

			vk->transitionImageLayout(
				cmd,
				vk->getSwapchainImage(imageIndex),
				vk->getSwapchainFormat(),
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

			// Begin dynamic rendering (single color + depth)
			VkRenderingAttachmentInfo colorAttachment{};
			colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			colorAttachment.imageView = vk->getSwapchainImageView(imageIndex);
			colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };

            VkRenderingAttachmentInfo depthAttachment{};
            depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            depthAttachment.imageView = depthRT[currentFrame].view;
            depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            depthAttachment.clearValue.depthStencil = {1.0f, 0};

			VkRenderingInfo renderInfo{};
			renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
			renderInfo.renderArea.offset = {0, 0};
			renderInfo.renderArea.extent = vk->getSwapchainExtent();
			renderInfo.layerCount = 1;
			renderInfo.colorAttachmentCount = 1;
			renderInfo.pColorAttachments = &colorAttachment;
			renderInfo.pDepthAttachment = &depthAttachment;

			vkCmdBeginRendering(cmd, &renderInfo);

			// Dynamic viewport/scissor
			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)vk->getSwapchainExtent().width;
			viewport.height = (float)vk->getSwapchainExtent().height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(cmd, 0, 1, &viewport);

			VkRect2D scissor{};
			scissor.offset = {0, 0};
			scissor.extent = vk->getSwapchainExtent();
			vkCmdSetScissor(cmd, 0, 1, &scissor);

            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, forwardPipelienTmp->getPipeline());
            
            VkDescriptorSet cameraDescriptorSet = globalSets[currentFrame];
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, forwardPipelienTmp->getPipelineLayout(), 0, 1, &cameraDescriptorSet, 0, nullptr);

			for ( auto it : renderGeometry )
			{
				std::shared_ptr<StaticMesh> staticMesh = it->getMesh();
                std::shared_ptr<Material> material = it->getMaterial();
				

				VkBuffer vertexBuffer;
				VkBuffer indexBuffer;
				uint32 indexCount;
				staticMesh->getBuffers(vertexBuffer, indexBuffer, indexCount);

				VkBuffer vertexBuffers[] = {vertexBuffer};
				VkDeviceSize offsets[] = {0};
				vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
				vkCmdBindIndexBuffer(cmd, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
				
				VkDescriptorSet materialDescriptorSet = getOrCreateMaterialSet(material);    
                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, forwardPipelienTmp->getPipelineLayout(), 1, 1, &materialDescriptorSet, 0, nullptr);

                Transform transform = it->getTransform_World();
				glm::mat4 model = transform.getTransformMatrix();
                vkCmdPushConstants(cmd, forwardPipelienTmp->getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &model);

				vkCmdDrawIndexed(cmd, indexCount, 1, 0, 0, 0);
			}

			vkCmdEndRendering(cmd);

			vk->transitionImageLayout(
				cmd,
				vk->getSwapchainImage(imageIndex),
				vk->getSwapchainFormat(),
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		}
	);
}
