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

EAssetLoadingState StaticMesh::Load_Internal()
{ 
	LOG_INDENT;
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(EnginePtr->GetAssetManager()->FindAsset(MeshPath), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	
	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		LOG(Error, "Failed to load mesh {}, with {}", GetName(), importer.GetErrorString());
		return EAssetLoadingState::Invalid;
	}

	ProcessNode(scene->mRootNode, scene);

	CreateVertexBuffer();
	CreateIndexBuffer();

	LOG(Log, "Loaded mesh '{}', verticies: {}, indicies: {}", GetName(), Data.Vertices.size(), Data.Indices.size());
	
	return EAssetLoadingState::Loaded;
}

void StaticMesh::CreateVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(Vertex) * Data.Vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	EnginePtr->GetVulkanCore()->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(EnginePtr->GetVulkanCore()->GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, Data.Vertices.data(), (size_t) bufferSize);
	vkUnmapMemory(EnginePtr->GetVulkanCore()->GetDevice(), stagingBufferMemory);

	EnginePtr->GetVulkanCore()->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VertexBuffer, VertexBufferMemory);

	EnginePtr->GetVulkanCore()->CopyBuffer(stagingBuffer, VertexBuffer, bufferSize);

	vkDestroyBuffer(EnginePtr->GetVulkanCore()->GetDevice(), stagingBuffer, nullptr);
    vkFreeMemory(EnginePtr->GetVulkanCore()->GetDevice(), stagingBufferMemory, nullptr);
}

void StaticMesh::CreateIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(Data.Indices[0]) * Data.Indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	EnginePtr->GetVulkanCore()->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(EnginePtr->GetVulkanCore()->GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, Data.Indices.data(), (size_t) bufferSize);
	vkUnmapMemory(EnginePtr->GetVulkanCore()->GetDevice(), stagingBufferMemory);


	EnginePtr->GetVulkanCore()->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, IndexBuffer, IndexBufferMemory);

	EnginePtr->GetVulkanCore()->CopyBuffer(stagingBuffer, IndexBuffer, bufferSize);

	vkDestroyBuffer(EnginePtr->GetVulkanCore()->GetDevice(), stagingBuffer, nullptr);
    vkFreeMemory(EnginePtr->GetVulkanCore()->GetDevice(), stagingBufferMemory, nullptr);
}

void StaticMesh::Cleanup_Internal()
{
	vkDestroyBuffer(EnginePtr->GetVulkanCore()->GetDevice(), VertexBuffer, nullptr);
	vkFreeMemory(EnginePtr->GetVulkanCore()->GetDevice(), VertexBufferMemory, nullptr);

	vkDestroyBuffer(EnginePtr->GetVulkanCore()->GetDevice(), IndexBuffer, nullptr);
	vkFreeMemory(EnginePtr->GetVulkanCore()->GetDevice(), IndexBufferMemory, nullptr);
}

void StaticMesh::ProcessNode(struct aiNode* node, const struct aiScene* scene)
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
		Data.Vertices.reserve(mesh->mNumVertices + Data.Vertices.size());
		for(uint32 i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			glm::vec3 vector;
			if ( mesh->mVertices )
			{
				vector.x = mesh->mVertices[i].x;
				vector.y = mesh->mVertices[i].y;
				vector.z = mesh->mVertices[i].z;
				vertex.Position = vector;
			}
		
			if ( mesh->mNormals )
			{
				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.Normal = vector;
			}
			if(mesh->mTextureCoords[0])
			{
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x; 
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}
			else
			{
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);
			}
			if ( mesh->mTangents )
			{
				vector.x = mesh->mTangents[i].x;
				vector.y = mesh->mTangents[i].y;
				vector.z = mesh->mTangents[i].z;
				vertex.Tangent = vector;
			}
			if ( mesh->mBitangents )
			{
				vector.x = mesh->mBitangents[i].x;
				vector.y = mesh->mBitangents[i].y;
				vector.z = mesh->mBitangents[i].z;
				vertex.Bitangent = vector;
			}
			if ( mesh->HasVertexColors(0) )
			{
				vertex.Color = glm::vec4(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b, mesh->mColors[0][i].a);
			}
			Data.Vertices.push_back(vertex);
		}
		

		for(uint32 i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			Data.Indices.reserve(face.mNumIndices + Data.Indices.size());
			for(uint32 j = 0; j < face.mNumIndices; j++)
			{
				Data.Indices.push_back(face.mIndices[j]);	
			}
		}
	}

	for(uint32 i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene);
	}
}