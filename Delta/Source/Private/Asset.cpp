#include "stdafx.h"
#include "Asset.h"

using namespace Delta;


void Asset::onDestroy()
{
	cleanup();
	Object::onDestroy();
}

EAssetLoadingState Asset::load()
{
	state = load_Internal();
	return state;
}

void Asset::cleanup()
{
	LOG(Log, "Cleanup asset '{}'", getName());
	cleanup_Internal();
	state = EAssetLoadingState::INVALID;
}