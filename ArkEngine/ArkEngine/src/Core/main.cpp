#include "Game.h"

#include "EngineInclude.h"


int main()
{
	//create instance of the game and engine
	Game* game = new Game();
	Engine::ArkEngine& engine = Engine::ArkEngine::GetInstance();
	engine.SetApplication(game);

	if (engine.Init(1280,720))
	{
		engine.Run();
	}

	engine.Destroy();

	return 0;

}