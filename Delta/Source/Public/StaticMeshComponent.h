#pragma once

#include "stdafx.h"
#include "ActorComponent.h"
#include "IRenderable.h"

namespace Delta
{

class StaticMeshComponent : public ActorComponent
{
public:
	template <typename... Args>
	StaticMeshComponent(const std::string& inMeshAsset, const std::string& inMaterialAsset, Args&&... args) :
		ActorComponent(std::forward<Args>(args)...),
		meshAssetPath(inMeshAsset),
		materialAssetPath(inMaterialAsset)
	{}

	const std::shared_ptr<class StaticMesh> getMesh() const { return mesh; }
	const std::shared_ptr<class Material> getMaterial() const { return material; }

protected:
	virtual bool initialize_Internal() override;
	virtual void onDestroy() override;
	
private:
	std::shared_ptr<class StaticMesh> mesh;
	
	std::shared_ptr<class Material> material;


	const std::string meshAssetPath = "";
	const std::string materialAssetPath = "";
};

}