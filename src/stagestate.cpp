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
#include "boss.h"

StageState::StageState() {
    /* 1. BACKGROUNDS (Parallax usando classe FundoInfinito) */
    // Como a tela tem 1200x900 e o bg original tem 320x192, precisamos escalar
    float scaleX = 1200.0f / 320.0f; // 3.75f
    float scaleY = 900.0f / 192.0f;  // ~4.69f

    GameObject* bg1 = new GameObject();
    bg1->box.y = -120.0f; // Ajuste vertical para a camada da frente
    bg1->AddComponent(new FundoInfinito(*bg1, "map/fase2/bg4fase2.png", 0.05f, scaleX, scaleY));
    AddObject(bg1);

    GameObject* bg2 = new GameObject();
    bg2->box.y = -120.0f; // Ajuste vertical para a camada da frente
    bg2->AddComponent(new FundoInfinito(*bg2, "map/fase2/bg3fase2.png", 0.15f, scaleX, scaleY));
    AddObject(bg2);

    GameObject* bg3 = new GameObject();
    bg3->AddComponent(new FundoInfinito(*bg3, "map/fase2/bg2fase2.png", 0.30f, scaleX, scaleY));
    AddObject(bg3);

    GameObject* bg4 = new GameObject();
    bg4->box.y = 180.0f; // Ajuste vertical para a camada da frente
    bg4->AddComponent(new FundoInfinito(*bg4, "map/fase2/bg1fase2.png", 0.50f, scaleX, scaleY));
    AddObject(bg4);

    GameObject* bg5 = new GameObject();
    bg5->box.y = 310.0f; // Ajuste vertical para a camada da frente
    bg5->AddComponent(new FundoInfinito(*bg5, "map/fase2/bg1fase2.png", 0.50f, scaleX+0.50f, scaleY));
    AddObject(bg5);

    GameObject* bg6 = new GameObject();
    bg6->box.y = 440.0f; // Ajuste vertical para a camada da frente
    bg6->AddComponent(new FundoInfinito(*bg6, "map/fase2/bg1fase2.png", 0.50f, scaleX+1.0f, scaleY));
    AddObject(bg6);

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
    playerObject->box.y = 7 * tileSet->GetTileHeight(); // Cai até o primeiro tile sólido

    // Usando o sprite que foi dimensionou em Character.cpp
    Character* playerCharacter = new Character(*playerObject, "img/sarue.png", tileMap);
    playerObject->AddComponent(playerCharacter);

    PlayerController* playerController = new PlayerController(*playerObject);
    playerObject->AddComponent(playerController);
    AddObject(playerObject);

   // COORDENADAS DOS INIMIGOS
    std::vector<std::pair<int, int>> posPombos = {
        {13, 5}, {20, 5}, {26, 4}, {32, 4}, {37, 5}
    };

    std::vector<std::pair<int, int>> posGatos = {
        {11, 4}, {71,7}, {48, 7}, {53, 8}, {63, 10}, {81, 9}, 
        {84, 7}, {80, 4}, {85, 4}, {88, 3}
    };

    //FRUTAS
    std::vector<std::pair<int, int>> posFrutas = {
        {90, 10}, {91, 7}, {80, 2}
    };

    // GERANDO OS POMBOS (Passando o tileMap)
    for (auto& pos : posPombos) {
        GameObject* pomboObj = new GameObject();
        float spawnX = pos.first * tileSet->GetTileWidth();
        float spawnY = pos.second * tileSet->GetTileHeight();
        
        Enemy* pombo = new Enemy(*pomboObj, spawnX, spawnY, tileMap);
        pomboObj->AddComponent(pombo);
        AddObject(pomboObj);
    }

    // GERANDO OS GATOS (Passando o tileMap)
    for (auto& pos : posGatos) {
        GameObject* gatoObj = new GameObject();
        float spawnX = pos.first * tileSet->GetTileWidth();
        float spawnY = pos.second * tileSet->GetTileHeight();

        Gato* gato = new Gato(*gatoObj, spawnX, spawnY, tileMap);
        gatoObj->AddComponent(gato);
        AddObject(gatoObj);
    }

    // GERANDO AS FRUTAS (Passando o tileMap)
    for (auto& pos : posFrutas) {
        GameObject* fruitObj = new GameObject();
        
        // Converte a coordenada do grid (X, Y) para pixels no mundo
        float spawnX = pos.first * tileSet->GetTileWidth();
        float spawnY = pos.second * tileSet->GetTileHeight();

        Fruit* fruit = new Fruit(*fruitObj, spawnX, spawnY);
        fruitObj->AddComponent(fruit);
        AddObject(fruitObj);
    }

    /* 5. HUD E CÂMERA */
    Camera::Follow(playerObject);

    GameObject* hudObject = new GameObject();
    HUD* hudUI = new HUD(*hudObject);
    hudObject->AddComponent(hudUI);
    AddObject(hudObject);

    /* 6. CHEFE FINAL */
    GameObject* bossObj = new GameObject();
    // Posiciona ele fora da tela na direita (Tile 96), esperando o gatilho
    float bossX = 96 * tileSet->GetTileWidth();
    float bossY = 0 * tileSet->GetTileHeight();
    Boss* boss = new Boss(*bossObj, bossX, bossY, tileMap);
    bossObj->AddComponent(boss);
    AddObject(bossObj);
}

StageState::~StageState() {}

void StageState::Start() {
    LoadAssets();
    StartArray();
    backgroundMusic.Play(-1);
}

void StageState::LoadAssets() {
    backgroundMusic.Open("audio/stagemusic.mp3");
}

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

        backgroundMusic.Stop(0);

        Game::GetInstance().Push(new EndState());
    }

    if (GameData::playerVictory) {
        popRequested = true;
        backgroundMusic.Stop(0);
        Game::GetInstance().Push(new EndState());
        return; // Encerra o frame imediatamente
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

    // =========================================================================
    // LIMITES DA CÂMERA (BORDAS DO MAPA)
    // =========================================================================
    if (tileMap != nullptr && tileSet != nullptr) {
        // Calcula o tamanho total do mapa em pixels lendo as informações do Tiled
        float mapWidthPixels = tileMap->GetWidth() * tileSet->GetTileWidth();
        float mapHeightPixels = tileMap->GetHeight() * tileSet->GetTileHeight();
        
        float screenWidth = 1200.0f;
        float screenHeight = 900.0f;

        // Limita a borda Esquerda e Direita
        if (mapWidthPixels > screenWidth) {
            if (Camera::pos.x < 0.0f) {
                Camera::pos.x = 0.0f;
            } else if (Camera::pos.x > mapWidthPixels - screenWidth) {
                Camera::pos.x = mapWidthPixels - screenWidth;
            }
        } else {
            // Se o mapa for menor que a tela (ex: tela de Boss), centraliza a câmera
            Camera::pos.x = (mapWidthPixels - screenWidth) / 2.0f;
        }

        // Limita a borda Superior e Inferior
        /*if (mapHeightPixels > screenHeight) {
            if (Camera::pos.y < 0.0f) {
                Camera::pos.y = 0.0f;
            } else if (Camera::pos.y > mapHeightPixels - screenHeight) {
                Camera::pos.y = mapHeightPixels - screenHeight;
            }
        } else {
            Camera::pos.y = (mapHeightPixels - screenHeight) / 2.0f;
        }*/

        // Limita APENAS a borda Inferior (O teto agora é infinito!)
            if (mapHeightPixels > screenHeight) {
                // A trava de (Camera::pos.y < 0.0f) foi removida para você poder subir o prédio!
                
                // Trava para não passar do chão
                if (Camera::pos.y > mapHeightPixels - screenHeight) {
                    Camera::pos.y = mapHeightPixels - screenHeight;
                }
            } else {
                Camera::pos.y = (mapHeightPixels - screenHeight) / 2.0f;
            }
    }
}

void StageState::Render() {
    RenderArray(); 
}

void StageState::Pause() {}

void StageState::Resume() {}