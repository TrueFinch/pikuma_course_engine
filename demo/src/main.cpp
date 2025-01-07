//
// Created by Vladimir Glushkov on 18.12.2024.
//

#include <iostream>
#include <SDL2/SDL.h>

#include <engine/coreModule/Game.h>

int main(int argc, char *argv[]) {
	pce::Game game;
	game.Initialize();
	game.Run();
	game.Destroy();

	return 0;
}
