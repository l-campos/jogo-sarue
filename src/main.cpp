#include "game.h"
#include "stagestate.h"
#include "titlestate.h"

int main(int argc, char** argv) {
    Game& game = Game::GetInstance();

    game.Push(new TitleState());
    game.Run();
    
    return 0;
}