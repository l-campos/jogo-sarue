#define INCLUDE_SDL_IMAGE
#define INCLUDE_SDL_MIXER
#define INCLUDE_SDL_TTF

#include <iostream>
#include <cstdlib>
#include <ctime>
#include "game.h"
#include "resources.h"
#include "stagestate.h"
#include "SDL_include.h"
#include "inputmanager.h"

using namespace std;

Game* Game::instance = nullptr;

Game& Game::GetInstance() {
    if (instance == nullptr) {
        instance = new Game("Lucas Fernandes - 180022563", 1200, 900);
    }
    return *instance;
}

Game::Game(string title, int width, int height) {
    if (instance != nullptr) {
        cerr << "Erro: Game já possui uma instância rodando!" << endl;
        return;
    } else {
        instance = this;
    }

    frameStart = 0;
    dt = 0.0f;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        cerr << "Erro ao inicializar SDL: " << SDL_GetError() << endl;
    }

    int imgFlags = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF;
    if (IMG_Init(imgFlags) == 0) {
        cerr << "Erro ao inicializar SDL_image: " << SDL_GetError() << endl;
    }

    int mixFlags = MIX_INIT_FLAC | MIX_INIT_OGG | MIX_INIT_MP3;
    if (Mix_Init(mixFlags) == 0) {
        cerr << "Erro ao inicializar SDL_mixer: " << SDL_GetError() << endl;
    }

    if (TTF_Init() != 0) {
        cerr << "Erro ao inicializar SDL_ttf: " << SDL_GetError() << endl;
    }

    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024) != 0) {
        cerr << "Erro no Mix_OpenAudio: " << SDL_GetError() << endl;
    }

    Mix_AllocateChannels(32);

    window = SDL_CreateWindow(
        title.c_str(), 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        width, 
        height, 
        0
    );

    if (window == nullptr) {
        cerr << "Erro ao criar janela: " << SDL_GetError() << endl;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        cerr << "Erro ao criar renderizador: " << SDL_GetError() << endl;
    }

    SDL_RenderSetLogicalSize(renderer, 800, 600);

    storedState = nullptr;

    srand(time(NULL));
}

Game::~Game() {
    if (storedState != nullptr) {
        delete storedState;
    }

    while (!stateStack.empty()) {
        stateStack.pop();
    }

    Resources::ClearImages();
    Resources::ClearMusics();
    Resources::ClearSounds();
    Resources::ClearFonts();

    Mix_CloseAudio();
    Mix_Quit();
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

State& Game::GetCurrentState() {
    return *stateStack.top();
}

SDL_Renderer* Game::GetRenderer() {
    return renderer;
}

void Game::CalculateDeltaTime() {
    int currentTime = SDL_GetTicks();
    dt = (currentTime - frameStart) / 1000.0f;
    frameStart = currentTime;
}

float Game::GetDeltaTime() {
    return dt;
}

void Game::Run() {
    if (storedState == nullptr) return;

    stateStack.push(std::unique_ptr<State>(storedState));
    stateStack.top()->Start();
    storedState = nullptr;

    while (!stateStack.empty() && !stateStack.top()->QuitRequested()) {
        if (stateStack.top()->PopRequested()) {
            stateStack.pop();
            if (!stateStack.empty()) {
                stateStack.top()->Resume();
            }
        }

        if (storedState != nullptr) {
            if (!stateStack.empty()) {
                stateStack.top()->Pause();
            }
            stateStack.emplace(storedState);
            stateStack.top()->Start();
            storedState = nullptr;
        }

        if (stateStack.empty()) {
            cout << "Nenhum estado ativo. Encerrando o jogo." << endl;
            break;
        }

        CalculateDeltaTime();
        InputManager::GetInstance().Update();

        stateStack.top()->Update(GetDeltaTime());
        // limpeza da imagem anterior
        // 1. Define a cor de limpeza. RGBA (0, 0, 0, 255) é Preto. 
        // (135, 206, 235, 255)
        SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255); 

        // 2. Apaga o quadro do frame anterior pintando tudo com a cor acima
        SDL_RenderClear(renderer);

        stateStack.top()->Render();

        SDL_RenderPresent(renderer);
        SDL_Delay(33);
    }

    Resources::ClearImages();
    Resources::ClearMusics();
    Resources::ClearSounds();
    Resources::ClearFonts();
}

void Game::Push(State* state) {
    storedState = state;
}