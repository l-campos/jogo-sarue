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
    // Escolha qual fase quer carregar mudando esta variável para testes (1 ou 2)
    //int faseAtual = 2; 

    /* 1. BACKGROUND (O fundo que se repete) */
    GameObject* bgObject = new GameObject();
    bgObject->box.y = 200.0f; // Ajusta este valor se a imagem ficar muito alta/baixa no ecrã

    FundoInfinito* fundo = nullptr;

    /*if (faseAtual == 1) {
        // Carrega o fundo da Fase 1
        fundo = new FundoInfinito(*bgObject, "map/fase1/bg1fase1.png");
    } else {
        // Carrega o fundo da Fase 2
        fundo = new FundoInfinito(*bgObject, "map/fase2/bg3Fase2.png");
    }*/
    
    // Carrega o fundo da Fase 2
    fundo = new FundoInfinito(*bgObject, "map/fase2/bg4Fase2.png");
    
    bgObject->AddComponent(fundo);
    AddObject(bgObject);


    /* 2. MAPA DE TILES (Carregamento Dinâmico via JSON) */
    GameObject* mapObject = new GameObject();
    TileSet* tileSet = new TileSet(32, 32, "map/fase2/tileset2.png");
    TileMap* tileMap = new TileMap(*mapObject, "map/fase2/SarueSagafase2.tmj", tileSet);

    mapObject->AddComponent(tileMap);
    AddObject(mapObject);


    /* 3. ENTIDADES (Saruê, HUD e Gerenciadores) */
    GameObject* playerObject = new GameObject();

    playerObject->box.x = 400.0f;
    playerObject->box.y = 100; // Nasce um pouco mais alto para cair no mapa
    Character* playerCharacter = new Character(*playerObject, "img/Player.png", tileMap);
    playerObject->AddComponent(playerCharacter);

    PlayerController* playerController = new PlayerController(*playerObject);
    playerObject->AddComponent(playerController);
    AddObject(playerObject);

    // Sistema de Câmera seguindo o Saruê através do novo mapa imenso
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
    RenderArray(); // Isso vai renderizar o Fundo, depois o Mapa, depois o Saruê na ordem certa!
}

void StageState::Pause() {}

void StageState::Resume() {}