#pragma once

#include "stdafx.h"
#include "Asset.h"

#include "MeshData.h"

#include <assimp/scene.h>

namespace Delta
{
class StaticMesh : public Asset
{
public:
	template <typename... Args>
	StaticMesh(const std::string& _MeshPath,Args&&... args) :
		Asset(std::forward<Args>(args)...),
		MeshPath(_MeshPath)
	{}

protected:
	virtual EAssetLoadingState Load_Internal() override;
	virtual void Cleanup_Internal() override;

private:
	void ProcessNode(aiNode* node, const aiScene* scene);

	void CreateVertexBuffer();
	void CreateIndexBuffer();

private:

	const std::string MeshPath;
	MeshData Data;

	VkBuffer VertexBuffer;
	VkDeviceMemory VertexBufferMemory;
	VkBuffer IndexBuffer;
	VkDeviceMemory IndexBufferMemory;
};

}