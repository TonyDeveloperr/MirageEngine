#include <iostream>
#include "./Game/Game.h"

int main(int argc, char* args[]) {

    Game game;

    game.Initialize();
    game.Run();
    game.Destroy();

    return 0;
}
