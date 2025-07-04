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
		meshPath(_MeshPath)
	{}

protected:
	virtual EAssetLoadingState load_Internal() override;
	virtual void cleanup_Internal() override;

private:
	void processNode(aiNode* node, const aiScene* scene);

	void createVertexBuffer();
	void createIndexBuffer();

private:

	const std::string meshPath;
	MeshData data;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
};

}