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
#include "hud.h"
#include "gato.h"
#include "fundo.h"
#include "spawner.h"

StageState::StageState() {

                        /*BACKGROUND*/
    GameObject* bgObject = new GameObject();
    bgObject->box.x = 0; 
    bgObject->box.y = 250; // Mantenha a altura que você já tinha configurado para alinhar o chão
    
    // Adiciona o nosso novo componente de looping
    FundoInfinito* fundo = new FundoInfinito(*bgObject, "img/fundo2.jpg");
    bgObject->AddComponent(fundo);
    
    

                        /*CHARACTER*/
    GameObject* playerObject = new GameObject();
    playerObject->box.x = 100;
    playerObject->box.y = 0;
    Character* playerCharacter = new Character(*playerObject, "img/Player.png");
    playerObject->AddComponent(playerCharacter);

                        /*CONTROLLER*/
    PlayerController* playerController = new PlayerController(*playerObject);
    playerObject->AddComponent(playerController);

                        /*INIMIGOS*/
    GameObject* spawnerObj = new GameObject();
    Spawner* gerador = new Spawner(*spawnerObj);
    spawnerObj->AddComponent(gerador);

    GameObject* hudObject = new GameObject();
    HUD* hudUI = new HUD(*hudObject);
    hudObject->AddComponent(hudUI);

    GameObject* gatoObject = new GameObject();
    Gato* gato = new Gato(*gatoObject, 800.0f, 450.0f);
    gatoObject->AddComponent(gato);
    
    Camera::Follow(playerObject);
    AddObject(bgObject);
    AddObject(playerObject);
    AddObject(hudObject);
    AddObject(gatoObject);
    AddObject(spawnerObj);
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