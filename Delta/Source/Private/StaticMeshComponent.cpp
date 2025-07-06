#include "stdafx.h"
#include "StaticMeshComponent.h"
#include "Engine.h"
#include "AssetManager.h"
#include "StaticMesh.h"
#include "Material.h"

using namespace Delta;

bool StaticMeshComponent::initialize_Internal()
{
	if ( !ActorComponent::initialize_Internal() )
		return false;

	mesh = engine->getAssetManager()->findOrLoad<StaticMesh>(meshAssetPath, meshAssetPath);
	material = engine->getAssetManager()->findOrLoad<Material>(materialAssetPath, materialAssetPath);


	return true;
}

void StaticMeshComponent::onDestroy()
{

	ActorComponent::onDestroy();
}

