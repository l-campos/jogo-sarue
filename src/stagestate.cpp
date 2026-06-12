#define INCLUDE_SDL_MIXER

#include <algorithm>
#include <cmath>
#include "SDL_include.h"
#include "stagestate.h"
#include "spriterenderer.h"
#include "zombie.h"
#include "tilemap.h"
#include "tileset.h"
#include "inputmanager.h"
#include "camera.h"
#include "character.h"
#include "playercontroller.h"
#include "collision.h"
#include "collider.h"
#include "aicontroller.h"
#include "gamedata.h"
#include "endstate.h"
#include "enemy.h"
#include "game.h"
#include "fruit.h"

StageState::StageState() {

                        /*BACKGROUND*/
    GameObject* bgObject = new GameObject();
    bgObject->box.x = 0; 
    bgObject->box.y = 0;
    SpriteRenderer* bgSprite = new SpriteRenderer(*bgObject, "img/FUNDO.jpg");
    bgObject->AddComponent(bgSprite);
    

                        /*CHARACTER*/
    GameObject* playerObject = new GameObject();
    playerObject->box.x = 100;
    playerObject->box.y = 0;
    Character* playerCharacter = new Character(*playerObject, "img/Player.png");
    playerObject->AddComponent(playerCharacter);

                        /*CONTROLLER*/
    PlayerController* playerController = new PlayerController(*playerObject);
    playerObject->AddComponent(playerController);

                        /*POMBO*/
    GameObject* pomboObject = new GameObject();
    Enemy* pombo = new Enemy(*pomboObject, 400.0f, 100.0f);
    pomboObject->AddComponent(pombo);

    GameObject* pomboObject2 = new GameObject();
    Enemy* pombo2 = new Enemy(*pomboObject2, 900.0f, 100.0f);
    pomboObject2->AddComponent(pombo2);

    GameObject* pomboObject3 = new GameObject();
    Enemy* pombo3 = new Enemy(*pomboObject3, 500.0f, 100.0f);
    pomboObject3->AddComponent(pombo3);

    GameObject* pomboObject4 = new GameObject();
    Enemy* pombo4 = new Enemy(*pomboObject4, 200.0f, 100.0f);
    pomboObject4->AddComponent(pombo4);

    GameObject* pomboObject5 = new GameObject();
    Enemy* pombo5 = new Enemy(*pomboObject5, 1000.0f, 100.0f);
    pomboObject5->AddComponent(pombo5);

    GameObject* frutaObject = new GameObject();
    Fruit* fruta = new Fruit(*frutaObject, 600.0f, 450.0f);
    frutaObject->AddComponent(fruta);
    
    GameObject* frutaObject2 = new GameObject();
    Fruit* fruta2 = new Fruit(*frutaObject2, 100.0f, 450.0f);
    frutaObject2->AddComponent(fruta2);
    
    GameObject* frutaObject3 = new GameObject();
    Fruit* fruta3 = new Fruit(*frutaObject3, 1000.0f, 450.0f);
    frutaObject3->AddComponent(fruta3);
    
    Camera::Follow(playerObject);
    AddObject(bgObject);
    AddObject(playerObject);
    AddObject(pomboObject);
    AddObject(pomboObject2);
    AddObject(pomboObject3);
    AddObject(pomboObject4);
    AddObject(pomboObject5);
    AddObject(frutaObject);
    AddObject(frutaObject2);
    AddObject(frutaObject3);
}

StageState::~StageState() {}

void StageState::Start() {
    LoadAssets();
    StartArray();
}

void StageState::LoadAssets(){}

void StageState::Update(float dt){
    InputManager& input = InputManager::GetInstance();
    
    if (input.QuitRequested()) {
        quitRequested = true;
    }

    if (input.KeyPress(ESCAPE_KEY)) {
        popRequested = true;
        backgroundMusic.Stop(0);
    }
    
    UpdateArray(dt);

    if (Character::player == nullptr) {
        GameData::playerVictory = false;
        popRequested = true;
        Game::GetInstance().Push(new EndState());
    }

    for (unsigned i = 0; i < objectArray.size(); i++) {
        for (unsigned j = i + 1; j < objectArray.size(); j++) {
            Collider* colliderA = objectArray[i]->GetComponent<Collider>();
            Collider* colliderB = objectArray[j]->GetComponent<Collider>();
            
            if (colliderA != nullptr && colliderB != nullptr) {
                float angleA = objectArray[i]->angleDeg * (M_PI / 180.0f);
                float angleB = objectArray[j]->angleDeg * (M_PI / 180.0f);

                if (Collision::IsColliding(colliderA->box, colliderB->box, angleA, angleB)) {
                    objectArray[i]->NotifyCollision(*objectArray[j]);
                    objectArray[j]->NotifyCollision(*objectArray[i]);
                }
            }
        }
    }

    for (int i = 0; i < (int)objectArray.size(); i++) {
        if (objectArray[i]->IsDead()) {
            objectArray.erase(objectArray.begin() + i);
            i--;
        }
    }
    
    Camera::Update(dt);
}

void StageState::Render() {

    std::sort(objectArray.begin(), objectArray.end(), 
        [](const std::shared_ptr<GameObject>& a, const std::shared_ptr<GameObject>& b) {
            return a->box.y < b->box.y; 
        }
    );

    RenderArray();
}

void StageState::Pause() {}

void StageState::Resume() {}