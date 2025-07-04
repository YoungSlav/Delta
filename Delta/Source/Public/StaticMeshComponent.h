#pragma once

#include "stdafx.h"
#include "ActorComponent.h"
#include "IRenderable.h"

namespace Delta
{

class StaticMeshComponent : public ActorComponent, public IRenderable
{
public:
	template <typename... Args>
	StaticMeshComponent(const std::string& MeshPath, Args&&... args) :
		ActorComponent(std::forward<Args>(args)...)
	{}

	void setMesh( std::shared_ptr<class StaticMesh> _mesh);
	void setMaterial( std::shared_ptr<class Material> _material);

	std::shared_ptr<class StaticMesh> getMesh() const;
	std::shared_ptr<class Material> getMaterial() const;

protected:

private:

private:
	std::shared_ptr<class StaticMesh> mesh;
	std::shared_ptr<class Material> material;
};

}