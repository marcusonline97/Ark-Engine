#include "Game.h"

#include "EngineInclude.h"
#include "Logger.h"

int main()
{
	Logging::EnableAllLevels();
	Logging::Init() << "Initialized Logger";
	//create instance of the game and engine
	Game* game = new Game();
	Engine::ArkEngine& engine = Engine::ArkEngine::GetInstance();
	engine.SetApplication(game);

	if (engine.Init(1280,720))
	{
		engine.Run();
	}
	else
	{
		Logging::Fatal() << "Engine Failed To Initialize";
	}
	
	engine.Destroy();
	Logging::Shutdown();

	return 0;

}