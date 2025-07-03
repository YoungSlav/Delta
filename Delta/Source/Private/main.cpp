
#include "stdafx.h"
#include "Engine.h"
#include "Material.h"
#include "Renderer.h"
#include "AssetManager.h"
unsigned int DelegateHandle::CURRENT_ID = 0;


int main()
{
	DeltaLog::Init("DeltaApp.log");
	LOG(Log, "Starting Delta application");

	try
	{
		std::shared_ptr<Delta::Engine> Engine(new Delta::Engine("Delta Engine"));
	
		Engine->Initialize();

		auto TempMaterialPtr = Engine->GetAssetManager()->FindOrLoad<Delta::Material>("TestMaterial", "Shaders\\triangle");

		Engine->GetRenderer()->TempMaterialPtr = TempMaterialPtr;

		Engine->GameLoop();
	}
	catch (const std::exception& e)
	{
		LOG(Fatal, e.what());
		return EXIT_FAILURE;
	}

	LOG(Log, "Application exit");

	return EXIT_SUCCESS;
}