
#include "stdafx.h"
unsigned int DelegateHandle::CURRENT_ID = 0;


#include "Engine.h"
#include "Material.h"
#include "VulkanCore.h"
#include "AssetManager.h"
#include "StaticMesh.h"
#include "Player.h"


int main()
{
	DeltaLog::init("DeltaApp.log");
	LOG(Log, "Starting Delta application");

	try
	{
		std::shared_ptr<Delta::Engine> Engine(new Delta::Engine("Delta Engine"));
	
		Engine->initialize();

		Engine->spawn<Delta::Player>("FirstPersonPlayer");


		auto TempMaterialPtr = Engine->getAssetManager()->findOrLoad<Delta::Material>("TestMaterial", "Shaders\\triangle");

		auto TestMesh = Engine->getAssetManager()->findOrLoad<Delta::StaticMesh>("TestMesh", "primitives\\triangle.fbx");

		Engine->getVulkanCore()->tempMaterialPtr = TempMaterialPtr;

		Engine->gameLoop();
	}
	catch (const std::exception& e)
	{
		LOG(Fatal, e.what());
		return EXIT_FAILURE;
	}

	LOG(Log, "Application exit");

	return EXIT_SUCCESS;
}