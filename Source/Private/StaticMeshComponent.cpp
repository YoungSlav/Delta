#include "stdafx.h"
#include "StaticMeshComponent.h"
#include "Engine.h"
#include "AssetManager.h"
#include "StaticMesh.h"
#include "Pipeline.h"
#include "Material.h"
#include "Renderer.h"

using namespace Delta;

bool StaticMeshComponent::initialize_Internal()
{
	if ( !ActorComponent::initialize_Internal() )
		return false;

	mesh = engine->getAssetManager()->findOrLoad<StaticMesh>(meshAssetPath, meshAssetPath);
	// Pipeline is owned by Renderer; just create material data
	material = spawn<Material>(texturePath + "_material", texturePath);

	return true;
}

void StaticMeshComponent::onDestroy()
{

	ActorComponent::onDestroy();
}
