#include "stdafx.h"
#include "StaticMeshComponent.h"
#include "Engine.h"
#include "AssetManager.h"
#include "StaticMesh.h"
#include "Pipeline.h"
#include "Material.h"

using namespace Delta;

bool StaticMeshComponent::initialize_Internal()
{
	if ( !ActorComponent::initialize_Internal() )
		return false;

	mesh = engine->getAssetManager()->findOrLoad<StaticMesh>(meshAssetPath, meshAssetPath);
	pipeline = engine->getAssetManager()->findOrLoad<Pipeline>(pipelineAssetPath, pipelineAssetPath);
	material = spawn<Material>(texturePath + "_material", pipeline, texturePath);

	return true;
}

void StaticMeshComponent::onDestroy()
{

	ActorComponent::onDestroy();
}

