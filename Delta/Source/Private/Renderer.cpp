#include "stdafx.h"
#include "Renderer.h"
#include "Engine.h"
#include "Scene.h"
#include "StaticMeshComponent.h"
#include "StaticMesh.h"
#include "Material.h"
#include "Camera.h"
#include "Window.h"
#include "Transform.h"
#include "VulkanCore.h"

using namespace Delta;

bool Renderer::initialize_Internal()
{
	createCameraUniformBuffer();
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
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroyBuffer(engine->getVulkanCore()->getDevice(), cameraUniformBuffers[i], nullptr);
		vkFreeMemory(engine->getVulkanCore()->getDevice(), cameraUniformBuffersMemory[i], nullptr);
	}
}

void Renderer::updateCameraUniformBuffer(const CameraInfo& cameraInfo, uint32 currentImage)
{
	memcpy(cameraUniformBuffersMapped[currentImage], &cameraInfo, sizeof(cameraInfo));
}

void Renderer::drawFrame(const std::shared_ptr<class Scene> scene)
{
	engine->getVulkanCore()->drawFrame(
		[=](VkCommandBuffer commandBuffer, uint32 currentFrame)
		{
			CameraInfo cameraInfo;
			scene->getCameraInfo(cameraInfo, engine->getWindow()->getViewportSize());

			updateCameraUniformBuffer(cameraInfo, currentFrame);

			std::vector<std::shared_ptr<StaticMeshComponent>> renderGeometry;
			scene->getRenderGeometry(renderGeometry);

			for ( auto it : renderGeometry )
			{
				std::shared_ptr<StaticMesh> staticMesh = it->getMesh();
				std::shared_ptr<Material> material = it->getMaterial();
				Transform transform = it->getTransform_World();

				glm::mat4 model = transform.getTransformMatrix();
				VkBuffer vertexBuffer;
				VkBuffer indexBuffer;
				uint32 indexCount;

				staticMesh->getBuffers(vertexBuffer, indexBuffer, indexCount);

				VkBuffer vertexBuffers[] = {vertexBuffer};
				VkDeviceSize offsets[] = {0};
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
				vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

				VkDescriptorSet descriptorSet = material->getDescriptorSet(currentFrame);

				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->getPipelineLayout(), 0, 1, &descriptorSet, 0, nullptr);

				vkCmdPushConstants(commandBuffer, material->getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &model);

				vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indexCount), 1, 0, 0, 0);

				
			}

		}
	);
}

