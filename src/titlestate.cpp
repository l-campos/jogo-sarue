#include "titlestate.h"
#include "stagestate.h"
#include "game.h"
#include "inputmanager.h"
#include "spriterenderer.h"
#include "camera.h"

TitleState::TitleState(){
    GameObject* titleObject = new GameObject();
    SpriteRenderer* titleSprite = new SpriteRenderer(*titleObject, "img/Title.png");
    titleSprite->SetScale(3.75, 4.689);
    titleObject->AddComponent(titleSprite);
    AddObject(titleObject);
}

TitleState::~TitleState(){
}

void TitleState::LoadAssets(){
    backgroundMusic.Open("audio/titlemusic.mp3");
}

void TitleState::Update(float dt){
    InputManager& input = InputManager::GetInstance();
    
    if (input.KeyPress(ESCAPE_KEY)){
        quitRequested = true;
    }
    if (input.KeyPress(SPACE_KEY)){
        // NOVO: Para a música do menu ao entrar no jogo
        backgroundMusic.Stop(0); 
        Game::GetInstance().Push(new StageState());
    }

    UpdateArray(dt);
}

void TitleState::Render(){
    RenderArray();
}

void TitleState::Start(){
    LoadAssets();
    StartArray();
    backgroundMusic.Play(-1);
}

void TitleState::Pause(){}

void TitleState::Resume(){
    Camera::pos = {0, 0};
}
