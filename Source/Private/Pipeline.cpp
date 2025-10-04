#include "stdafx.h"
#include "Pipeline.h"
#include "Engine.h"
#include "VulkanCore.h"
#include "AssetManager.h"
#include "MeshData.h"
#include <fstream>

#include "Camera.h"
#include "Renderer.h"

using namespace Delta;


EAssetLoadingState Pipeline::load_Internal()
{
	createDescriptorSetLayout();
	createDescriptorSets();
	createGraphicsPipeline();
	return EAssetLoadingState::LOADED;
}

void Pipeline::cleanup_Internal()
{
	vkFreeDescriptorSets(engine->getVulkanCore()->getDevice(), engine->getVulkanCore()->getDescriptorPool(), static_cast<uint32_t>(globalDescriptorSets.size()), globalDescriptorSets.data());
	vkDestroyPipeline(engine->getVulkanCore()->getDevice(), pipeline, nullptr);
	vkDestroyPipelineLayout(engine->getVulkanCore()->getDevice(), pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(engine->getVulkanCore()->getDevice(), globalDescriptorSetLayout, nullptr);
	vkDestroyDescriptorSetLayout(engine->getVulkanCore()->getDevice(), materialDescriptorSetLayout, nullptr);
}

void Pipeline::createDescriptorSetLayout()
{
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;

		if ( vkCreateDescriptorSetLayout(engine->getVulkanCore()->getDevice(), &layoutInfo, nullptr, &globalDescriptorSetLayout) != VK_SUCCESS )
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}
	{
		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 0;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &samplerLayoutBinding;

		if ( vkCreateDescriptorSetLayout(engine->getVulkanCore()->getDevice(), &layoutInfo, nullptr, &materialDescriptorSetLayout) != VK_SUCCESS )
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}
}

void Pipeline::createDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, globalDescriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = engine->getVulkanCore()->getDescriptorPool();
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	globalDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(engine->getVulkanCore()->getDevice(), &allocInfo, globalDescriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	std::vector<VkBuffer> cameraUniformBuffers  = engine->getRenderer()->getCameraUniformBuffers();

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = cameraUniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(CameraInfo);

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = globalDescriptorSets[i];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(engine->getVulkanCore()->getDevice(), 1, &descriptorWrite, 0, nullptr);
	}
}

void Pipeline::createGraphicsPipeline()
{
	const std::string vertShaderFile = engine->getAssetManager()->findAsset(shaderName + ".vert.spv");
	const std::string fragShaderFile = engine->getAssetManager()->findAsset(shaderName + ".frag.spv");

	LOG(Log, "Creating pipeline for material '{}'", getName());
	
	auto vertShaderCode = readFile(vertShaderFile);
	auto fragShaderCode = readFile(fragShaderFile);

	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] =
	{ vertShaderStageInfo, fragShaderStageInfo };


	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = config.cullMode;
	rasterizer.frontFace = config.frontFace;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = config.enableDepthTest ? VK_TRUE : VK_FALSE;
	depthStencil.depthWriteEnable = config.enableDepthWrite ? VK_TRUE : VK_FALSE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// One color blend attachment per color target
	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	uint32_t colorAttachmentCount = std::max<uint32_t>(1, (uint32_t)config.colorAttachmentFormats.size());
	colorBlendAttachments.resize(colorAttachmentCount);
	for (auto& a : colorBlendAttachments)
	{
		a.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		a.blendEnable = VK_FALSE;
	}

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = colorAttachmentCount;
	colorBlending.pAttachments = colorBlendAttachments.data();
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	std::vector<VkDynamicState> dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(glm::mat4);

	std::array<VkDescriptorSetLayout, 2> setLayouts = { globalDescriptorSetLayout, materialDescriptorSetLayout };

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 2;
	pipelineLayoutInfo.pSetLayouts = setLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

	if (vkCreatePipelineLayout(engine->getVulkanCore()->getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}

	// Fill defaults if config was not set by caller
	if (config.colorAttachmentFormats.empty())
	{
		config.colorAttachmentFormats.push_back(engine->getVulkanCore()->getSwapchainFormat());
	}
	if (!config.depthAttachmentFormat.has_value())
	{
		config.depthAttachmentFormat = engine->getVulkanCore()->getDepthFormatPublic();
	}

	VkPipelineRenderingCreateInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	renderingInfo.colorAttachmentCount = static_cast<uint32_t>(config.colorAttachmentFormats.size());
	renderingInfo.pColorAttachmentFormats = config.colorAttachmentFormats.data();
	renderingInfo.depthAttachmentFormat = config.depthAttachmentFormat.value_or(VK_FORMAT_UNDEFINED);

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = &renderingInfo;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	// Topology from config
	inputAssembly.topology = config.topology;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = VK_NULL_HANDLE;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(engine->getVulkanCore()->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(engine->getVulkanCore()->getDevice(), fragShaderModule, nullptr);
	vkDestroyShaderModule(engine->getVulkanCore()->getDevice(), vertShaderModule, nullptr);
}

// Static helpers
Pipeline::Config Pipeline::MakeForwardConfig(std::shared_ptr<VulkanCore> vk)
{
	Pipeline::Config cfg{};
	cfg.colorAttachmentFormats = { vk->getSwapchainFormat() };
	cfg.depthAttachmentFormat = vk->getDepthFormatPublic();
	cfg.enableDepthTest = true;
	cfg.enableDepthWrite = true;
	cfg.cullMode = VK_CULL_MODE_BACK_BIT;
	cfg.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	cfg.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	return cfg;
}

Pipeline::Config Pipeline::MakeGeometryGBufferConfig(VkFormat albedoFormat, VkFormat normalFormat, std::optional<VkFormat> depthFormat)
{
	Pipeline::Config cfg{};
	cfg.colorAttachmentFormats = { albedoFormat, normalFormat };
	cfg.depthAttachmentFormat = depthFormat;
	cfg.enableDepthTest = true;
	cfg.enableDepthWrite = true;
	cfg.cullMode = VK_CULL_MODE_BACK_BIT;
	cfg.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	cfg.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	return cfg;
}

VkShaderModule Pipeline::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(engine->getVulkanCore()->getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}


std::vector<char> Pipeline::readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file: " + filename + " !");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}
