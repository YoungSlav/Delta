#include "stdafx.h"
#include "Asset.h"

using namespace Delta;


void Asset::OnDestroy()
{
	Cleanup();
	Object::OnDestroy();
}

EAssetLoadingState Asset::Load()
{
	State = Load_Internal();
	return State;
}

void Asset::Cleanup()
{
	LOG(Log, "Cleanup asset '{}'", GetName());
	Cleanup_Internal();
	State = EAssetLoadingState::Invalid;
}