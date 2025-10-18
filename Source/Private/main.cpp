
#include "stdafx.h"

unsigned int DelegateHandle::CURRENT_ID = 0;


#include "Engine.h"
#include "Pipeline.h"
#include "VulkanCore.h"
#include "AssetManager.h"
#include "StaticMesh.h"
#include "Player.h"
#include "Scene.h"
#include "StaticMeshComponent.h"
#include "Actor.h"
#include "DirectionalLight.h"

int main()
{
	DeltaLog::init("DeltaApp");
	LOG(Log, "Starting Delta application");
	
	try
	{
		std::shared_ptr<Delta::Engine> engine(new Delta::Engine("Delta Engine"));
	
		engine->initialize();

		auto scene = engine->spawn<Delta::Scene>("TestScene");
		engine->openScene(scene);

		auto player = scene->spawn<Delta::Player>("First person player");
		player->setLocation(glm::vec3(10.0f, 0.0f, 0.0f));
		player->setDirection(glm::vec3(0.0f) - glm::vec3(10.0f, 0.0f, 0.0f));

		scene->setCamera(player->getCamera());

		{
			auto actor = scene->spawn<Delta::Actor>("Test Actor");
			auto meshComp = actor->spawn<Delta::StaticMeshComponent>("Viking house", "viking_house/viking_room.obj", "viking_house/viking_room.png");
			meshComp->setScale(glm::vec3(10.0f));
			meshComp->setRotation(glm::vec3(-90.0f, 0.0f, 0.0f));
		}

		{
			auto dirLightActor = scene->spawn<Delta::Actor>("Dir ligth actor");
			
			auto lightComp = dirLightActor->spawn<Delta::DirectionalLight>("Directional light");
			lightComp->setLocation(glm::vec3(10.0f, 10.0f, 0.0f));
		}



		engine->gameLoop();
	}
	catch (const std::exception& e)
	{
		LOG(Fatal, e.what());
		return EXIT_FAILURE;
	}

	LOG(Log, "Application exit");

	return EXIT_SUCCESS;
}
