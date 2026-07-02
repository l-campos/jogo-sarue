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
#include "spawner.h"

StageState::StageState() {

                        /*TILEMAP (fase 2)*/
    // AJUSTAR os caminhos para onde o PNG e o .tmj estiverem no projeto.
    tileSet = new TileSet(32, 32, "img/tilezada2.png");

    GameObject* mapObject = new GameObject();
    mapObject->box.x = 0;
    mapObject->box.y = 0;
    tileMap = new TileMap(*mapObject, "map/fase2.tmj", tileSet);
    mapObject->AddComponent(tileMap);

                        /*CHARACTER*/
    GameObject* playerObject = new GameObject();
    // AJUSTADO: o mapa tem offsetX=192.667, entao o grid de tiles comeca em
    // worldX~193. O player em x=100 ficava a esquerda de TODOS os tiles,
    // sem chao sob os pes -- caia para sempre. Agora comeca em x=250,
    // dentro do mapa, e cai ate pousar no primeiro tile solido abaixo.
    playerObject->box.x = 250;
    playerObject->box.y = 0;
    Character* playerCharacter = new Character(*playerObject, "img/personagem.png");
    playerObject->AddComponent(playerCharacter);

                        /*CONTROLLER*/
    PlayerController* playerController = new PlayerController(*playerObject);
    playerObject->AddComponent(playerController);

                        /*INIMIGOS*/
    GameObject* spawnerObj = new GameObject();
    Spawner* gerador = new Spawner(*spawnerObj);
    spawnerObj->AddComponent(gerador);

                        /*HUD*/
    GameObject* hudObject = new GameObject();
    HUD* hudUI = new HUD(*hudObject);
    hudObject->AddComponent(hudUI);
        
    // NOTA: StageState::Render() ordena por box.y antes de desenhar.
    // mapObject tem box.y=0 -- renderiza primeiro (atras de tudo).
    Camera::Follow(playerObject);
    AddObject(hudObject);
    AddObject(mapObject);
    AddObject(playerObject);
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