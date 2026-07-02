#include "titlestate.h"
#include "stagestate.h"
#include "game.h"
#include "inputmanager.h"
#include "spriterenderer.h"
#include "camera.h"
#include "text.h"

TitleState::TitleState(){
    GameObject* titleObject = new GameObject();
    SpriteRenderer* titleSprite = new SpriteRenderer(*titleObject, "img/Title.png");
    titleSprite->SetScale(3.75, 4.689);
    titleObject->AddComponent(titleSprite);
    AddObject(titleObject);

    /*TEXTO PISCANDO*/
    textObject = new GameObject();
    textObject->box.x = 300;
    textObject->box.y = 400;

    SDL_Color white = {255, 255, 255, 255};
    Text* text = new Text(*textObject, "font/neodgm.ttf", 40, Text::SOLID, "Pressione ESPAÇO para iniciar", white);
    textObject->AddComponent(text);
    
}

TitleState::~TitleState(){
    delete textObject;
}

void TitleState::LoadAssets(){}

void TitleState::Update(float dt){
    InputManager& input = InputManager::GetInstance();
    
    if (input.KeyPress(ESCAPE_KEY)){
        quitRequested = true;
    }

    if (input.KeyPress(SPACE_KEY)){
        Game::GetInstance().Push(new StageState());
    }

    textTimer.Update(dt);
    if (textTimer.Get() >= 0.5f) {
        showText = !showText;
        textTimer.Restart();
    }

    UpdateArray(dt);
    textObject->Update(dt);
}

void TitleState::Render(){
    RenderArray();

    if (showText) {
        textObject->Render();
    }
}

void TitleState::Start(){
    LoadAssets();
    StartArray();
}

void TitleState::Pause(){}

void TitleState::Resume(){
    Camera::pos = {0, 0};
}