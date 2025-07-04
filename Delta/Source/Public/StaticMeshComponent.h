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
	StaticMeshComponent(Args&&... args) :
		ActorComponent(std::forward<Args>(args)...),
		IRenderable()
	{}

	void setMesh( std::shared_ptr<class StaticMesh> inMesh) { mesh = inMesh; }
	void setMaterial( std::shared_ptr<class Material> inMaterial) { material = inMaterial; }

	std::shared_ptr<class StaticMesh> getMesh() const { return mesh; }
	std::shared_ptr<class Material> getMaterial() const { return material; }

protected:

private:

private:
	std::shared_ptr<class StaticMesh> mesh;
	std::shared_ptr<class Material> material;
};

}