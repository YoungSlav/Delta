#include "stdafx.h"

#include "StaticMesh.h"
#include "Engine.h"
#include "AssetManager.h"
#include "VulkanCore.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include "assimp/color4.h"
#include <assimp/postprocess.h>

using namespace Delta;

EAssetLoadingState StaticMesh::load_Internal()
{ 
	LOG_INDENT;
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(engine->getAssetManager()->findAsset(meshPath), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	
	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		LOG(Error, "Failed to load mesh '{}', with {}", getName(), importer.GetErrorString());
		return EAssetLoadingState::INVALID;
	}

	processNode(scene->mRootNode, scene);

	createVertexBuffer();
	createIndexBuffer();

	LOG(Log, "Loaded mesh '{}', verticies: {}, indicies: {}", getName(), data.vertices.size(), data.indices.size());
	
	return EAssetLoadingState::LOADED;
}

void StaticMesh::getBuffers(VkBuffer& outVertexBuffer, VkBuffer& outIndexBuffer, uint32& outIndexCount) const
{
	outVertexBuffer = vertexBuffer;
	outIndexBuffer = indexBuffer;
	outIndexCount = static_cast<uint32>(data.indices.size());
}

void StaticMesh::createVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(Vertex) * data.vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	engine->getVulkanCore()->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* bufferData;
	vkMapMemory(engine->getVulkanCore()->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &bufferData);
		memcpy(bufferData, data.vertices.data(), (size_t) bufferSize);
	vkUnmapMemory(engine->getVulkanCore()->getDevice(), stagingBufferMemory);

	engine->getVulkanCore()->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

	engine->getVulkanCore()->copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

	vkDestroyBuffer(engine->getVulkanCore()->getDevice(), stagingBuffer, nullptr);
	vkFreeMemory(engine->getVulkanCore()->getDevice(), stagingBufferMemory, nullptr);
}

void StaticMesh::createIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(data.indices[0]) * data.indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	engine->getVulkanCore()->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* bufferData;
	vkMapMemory(engine->getVulkanCore()->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &bufferData);
		memcpy(bufferData, data.indices.data(), (size_t) bufferSize);
	vkUnmapMemory(engine->getVulkanCore()->getDevice(), stagingBufferMemory);


	engine->getVulkanCore()->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

	engine->getVulkanCore()->copyBuffer(stagingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(engine->getVulkanCore()->getDevice(), stagingBuffer, nullptr);
	vkFreeMemory(engine->getVulkanCore()->getDevice(), stagingBufferMemory, nullptr);
}

void StaticMesh::cleanup_Internal()
{
	vkDestroyBuffer(engine->getVulkanCore()->getDevice(), vertexBuffer, nullptr);
	vkFreeMemory(engine->getVulkanCore()->getDevice(), vertexBufferMemory, nullptr);

	vkDestroyBuffer(engine->getVulkanCore()->getDevice(), indexBuffer, nullptr);
	vkFreeMemory(engine->getVulkanCore()->getDevice(), indexBufferMemory, nullptr);
}

void StaticMesh::processNode(struct aiNode* node, const struct aiScene* scene)
{
	//LOG_INDENT;
	//LOG(Log, "Loading node '{}'", node->mName.C_Str());
	

	// process each mesh located at the current node
	for(uint32 i = 0; i < node->mNumMeshes; i++)
	{
		//LOG_INDENT;
		// the node object only contains indices to index the actual objects in the scene. 
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		if ( !mesh )
		{
			LOG(Error, "Failed to load mesh #{}", i);
			continue;
		}

		//LOG(Log, "Loading mesh #{}", i);

		//Log::LogMessage(ELog::Log"  -LoadingMesh: {}", mesh->mName.C_Str());
		// Walk through each of the mesh's vertices
		data.vertices.reserve(mesh->mNumVertices + data.vertices.size());
		for(uint32 i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			glm::vec3 vector;
			if ( mesh->mVertices )
			{
				vector.x = mesh->mVertices[i].x;
				vector.y = mesh->mVertices[i].y;
				vector.z = mesh->mVertices[i].z;
				vertex.position = vector;
			}
		
			if ( mesh->mNormals )
			{
				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.normal = vector;
			}
			if(mesh->mTextureCoords[0])
			{
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x; 
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.texCoords = vec;
			}
			else
			{
				vertex.texCoords = glm::vec2(0.0f, 0.0f);
			}
			if ( mesh->mTangents )
			{
				vector.x = mesh->mTangents[i].x;
				vector.y = mesh->mTangents[i].y;
				vector.z = mesh->mTangents[i].z;
				vertex.tangent = vector;
			}
			if ( mesh->mBitangents )
			{
				vector.x = mesh->mBitangents[i].x;
				vector.y = mesh->mBitangents[i].y;
				vector.z = mesh->mBitangents[i].z;
				vertex.bitangent = vector;
			}
			if ( mesh->HasVertexColors(0) )
			{
				vertex.color = glm::vec4(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b, mesh->mColors[0][i].a);
			}
			data.vertices.push_back(vertex);
		}
		

		for(uint32 i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			data.indices.reserve(face.mNumIndices + data.indices.size());
			for(uint32 j = 0; j < face.mNumIndices; j++)
			{
				data.indices.push_back(face.mIndices[j]);	
			}
		}
	}

	for(uint32 i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}