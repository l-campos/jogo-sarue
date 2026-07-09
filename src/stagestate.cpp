#define INCLUDE_SDL_MIXER

#include <algorithm>
#include <cmath>
#include "SDL_include.h"
#include "stagestate.h"
#include "spriterenderer.h"
#include "tilemap.h"
#include "tileset.h"
#include "inputmanager.h"
#include "camera.h"
#include "character.h"
#include "playercontroller.h"
#include "collision.h"
#include "collider.h"
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
    /* 1. BACKGROUNDS (Parallax usando classe FundoInfinito) */
    // Como a tela tem 1200x900 e o bg original tem 320x192, precisamos escalar
    float scaleX = 1200.0f / 320.0f; // 3.75f
    float scaleY = 900.0f / 192.0f;  // ~4.69f

    GameObject* bg1 = new GameObject();
    bg1->AddComponent(new FundoInfinito(*bg1, "map/fase2/bg4fase2.png", 0.05f, scaleX, scaleY));
    AddObject(bg1);

    GameObject* bg2 = new GameObject();
    bg2->AddComponent(new FundoInfinito(*bg2, "map/fase2/bg3fase2.png", 0.15f, scaleX, scaleY));
    AddObject(bg2);

    GameObject* bg3 = new GameObject();
    bg3->AddComponent(new FundoInfinito(*bg3, "map/fase2/bg2fase2.png", 0.30f, scaleX, scaleY));
    AddObject(bg3);


    GameObject* bg4 = new GameObject();
    bg4->box.y = 50.0f; // Ajuste vertical para a camada da frente
    bg4->AddComponent(new FundoInfinito(*bg4, "map/fase2/bg1fase2.png", 0.50f, scaleX, scaleY));
    AddObject(bg4);

    /* 2. MAPA DE TILES (Carregamento Dinâmico via JSON) */
    GameObject* mapObject = new GameObject();
    mapObject->box.x = 0;
    mapObject->box.y = 0;
    
    tileSet = new TileSet(32, 32, "map/fase2/tileset2.png");
    
    // A escala pertence ao TileSet, não ao TileMap!
    tileSet->SetScale(4.0f);
    
    tileMap = new TileMap(*mapObject, "map/fase2/fase2.tmj", tileSet);
    
    mapObject->AddComponent(tileMap);
    AddObject(mapObject);

    /* 3. ENTIDADES (Saruê) */
    GameObject* playerObject = new GameObject();

    // Para nascer na coluna 2, basta multiplicar pelo tamanho real do tile
    playerObject->box.x = 2 * tileSet->GetTileWidth(); 
    playerObject->box.y = 0; // Cai até o primeiro tile sólido

    // Usando o sprite que foi dimensionou em Character.cpp
    Character* playerCharacter = new Character(*playerObject, "img/sarue.png", tileMap);
    playerObject->AddComponent(playerCharacter);

    PlayerController* playerController = new PlayerController(*playerObject);
    playerObject->AddComponent(playerController);
    AddObject(playerObject);

    /* 4. INIMIGOS E ITENS (Recuperado: Mantém a IA gerando os inimigos) */
    GameObject* spawnerObj = new GameObject();
    Spawner* gerador = new Spawner(*spawnerObj);
    spawnerObj->AddComponent(gerador);
    AddObject(spawnerObj);

    /* 5. HUD E CÂMERA */
    Camera::Follow(playerObject);

    GameObject* hudObject = new GameObject();
    HUD* hudUI = new HUD(*hudObject);
    hudObject->AddComponent(hudUI);
    AddObject(hudObject);
}

StageState::~StageState() {}

void StageState::Start() {
    LoadAssets();
    StartArray();
}

void StageState::LoadAssets() {}

void StageState::Update(float dt) {
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
    RenderArray(); 
}

void StageState::Pause() {}

void StageState::Resume() {}