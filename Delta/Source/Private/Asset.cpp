#include "stdafx.h"
#include "Asset.h"

using namespace Delta;


void Asset::OnDestroy()
{
	Cleanup();
	Object::OnDestroy();
}

bool Asset::Load()
{
	bLoaded = Load_Internal();
	return bLoaded;
}

bool Asset::Load_Internal()
{
	return true;
}

void Asset::Cleanup()
{
	bLoaded = false;
}