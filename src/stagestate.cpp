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

    /* 1. BACKGROUNDS (Parallax de 4 Camadas) */
    // Quanto maior o valor, mais para BAIXO a imagem vai ficar na tela.
    float alturaFundo = 250.0f; 

    // Camada 1: Céu
    GameObject* bg1 = new GameObject();
    bg1->box.y = alturaFundo;
    bg1->AddComponent(new FundoInfinito(*bg1, "map/fase2/bg4Fase2.png", 0.05f));
    AddObject(bg1);

    // Camada 2: Construções/Montanhas distantes
    GameObject* bg2 = new GameObject();
    bg2->box.y = alturaFundo;
    bg2->AddComponent(new FundoInfinito(*bg2, "map/fase2/bg3Fase2.png", 0.3f));
    AddObject(bg2);

    // Camada 3: Árvores do meio
    GameObject* bg3 = new GameObject();
    bg3->box.y = alturaFundo;
    bg3->AddComponent(new FundoInfinito(*bg3, "map/fase2/bg2Fase2.png", 0.6f));
    AddObject(bg3);

    // Camada 4: Árvores da frente
    GameObject* bg4 = new GameObject();
    bg4->box.y = alturaFundo + 50.0f; // Ajuste vertical para a camada da frente
    bg4->AddComponent(new FundoInfinito(*bg4, "map/fase2/bg1Fase2.png", 0.85f));
    AddObject(bg4);

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