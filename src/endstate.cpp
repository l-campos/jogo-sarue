#include "endstate.h"
#include "gamedata.h"
#include "game.h"
#include "spriterenderer.h"
#include "inputmanager.h"
#include "camera.h"
#include "text.h"
#include "titlestate.h"
#include "stagestate.h"

EndState::EndState(){
    GameObject* bg = new GameObject();
    SpriteRenderer* sprite = nullptr;
    
    if (GameData::playerVictory) {
        sprite = new SpriteRenderer(*bg, "img/Win.png");
        sprite->SetScale(3.75, 4.689);
        backgroundMusic.Open("audio/titlemusic.mp3");
    } 
    
    else {
        sprite = new SpriteRenderer(*bg, "img/Lose.png");
        sprite->SetScale(3.75, 4.689);
        backgroundMusic.Open("audio/endStateLose.ogg");
    }

    bg->AddComponent(sprite);
    AddObject(bg);

    GameObject* textObj = new GameObject();
    textObj->box.x = 180;
    textObj->box.y = 440;
    SDL_Color color = {255, 255, 255, 255};

    Text* text = new Text(*textObj, "font/neodgm.ttf", 40, Text::BLENDED, "ESC para Sair ou ESPAÇO para Jogar Novamente", color); 
    textObj->AddComponent(text);
    AddObject(textObj);
}

EndState::~EndState(){}

void EndState::Update(float dt){
    InputManager& input = InputManager::GetInstance();

    if (input.KeyPress(ESCAPE_KEY)) {
        quitRequested = true;
    }
    if (input.KeyPress(SPACE_KEY)) {
        popRequested = true;
        
        // Faz a música de Vitória/Derrota parar imediatamente ao sair da tela
        backgroundMusic.Stop(0); 
        
        Game::GetInstance().Push(new StageState());
    }
    
    UpdateArray(dt);
}

void EndState::Render(){
    RenderArray();
}

void EndState::Start(){
    LoadAssets();
    StartArray();
    backgroundMusic.Play(-1);
}

void EndState::Pause() {}

void EndState::Resume() {
    Camera::pos = {0, 0};
}

void EndState::LoadAssets() {}