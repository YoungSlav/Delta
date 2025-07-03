#include "stdafx.h"

#include "StaticMesh.h"
#include "Engine.h"
#include "AssetManager.h"

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

	// process ASSIMP's root node recursively
	ProcessNode(scene->mRootNode, scene);

	LOG(Log, "Loaded mesh '{}', verticies: {}, indicies: {}", GetName(), Data.Vertices.size(), Data.Indices.size());
	
	return EAssetLoadingState::Loaded;
}

void StaticMesh::Cleanup_Internal()
{
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