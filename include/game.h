#ifndef GAME_H
#define GAME_H
#define INCLUDE_SDL

#include <string>
#include <stack>
#include <memory>
#include "SDL_include.h"

class State;

class Game {
public:
    Game(std::string title, int width, int height); 
    ~Game();
    
    static Game& GetInstance();
    SDL_Renderer* GetRenderer();
    State& GetCurrentState();

    void Push(State* state);
    
    void Run();
    
    float GetDeltaTime();
    SDL_Window* window;

private:
    void CalculateDeltaTime();
    int frameStart;
    float dt;
    
    static Game* instance; 
    
    SDL_Renderer* renderer;
    State* storedState;
    std::stack<std::unique_ptr<State>> stateStack;    
};

#endif