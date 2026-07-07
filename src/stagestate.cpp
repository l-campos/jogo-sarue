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
#include "spawner.h"
#include "parallaxbg.h"  // NOVO: background em parallax

StageState::StageState() {

                        /*BACKGROUND EM PARALLAX*/
    // O GameObject do background precisa ter box.y muito negativo para que o
    // sort por box.y em Render() o coloque SEMPRE no fundo (desenhado primeiro,
    // atrás de tudo). O componente usa cameraFollower=true e posições de tela,
    // então o valor de box.y não afeta onde aparece visualmente.
    GameObject* bgObject = new GameObject();
    bgObject->box.y = -10000.0f;
    ParallaxBackground* bg = new ParallaxBackground(*bgObject);
    bgObject->AddComponent(bg);

                        /*TILEMAP (fase 2)*/
    // AJUSTAR os caminhos para onde os arquivos ficam no projeto.
    tileSet = new TileSet(32, 32, "img/tileset2.png");

    GameObject* mapObject = new GameObject();
    mapObject->box.x = 0;
    mapObject->box.y = 0;
    tileMap = new TileMap(*mapObject, "map/fase2.tmj", tileSet);
    mapObject->AddComponent(tileMap);

    // ESCALA DO MAPA: padrão é 2x (definido em tileset.cpp).
    // Para aumentar ou diminuir, descomente e ajuste o valor abaixo.
    // Afeta render E colisão automaticamente — um único número controla tudo.
    tileMap->SetScale(4.0f); // 3x: tiles de 96px

                        /*CHARACTER*/
    GameObject* playerObject = new GameObject();
    // Posiciona o player em coordenadas de TILE, não de pixel fixo.
    // Razão: o pixel de mundo correspondente a cada tile muda com a escala.
    // Com escala 2x, tileToPixelX(2) ≈ 513px. Com 3x ≈ 770px. Ao usar
    // TileToPixelX, o spawn sempre cai dentro do mapa independente da escala,
    // evitando que o player nasça à esquerda de todos os tiles e caia para sempre.
    playerObject->box.x = tileMap->TileToPixelX(2); // coluna 2 do grid de tiles
    playerObject->box.y = 0;                         // cai até o primeiro tile sólido
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

    Camera::Follow(playerObject);

    // Ordem de AddObject reflete a ordem padrão de render (antes do sort).
    // Como Render() faz sort por box.y, o que importa é o valor de box.y:
    //   bgObject:    -10000  → renderiza primeiro (fundo)
    //   mapObject:       0   → renderiza antes do player
    //   playerObject: ~430   → renderiza na posição do mundo
    //   hudObject:       0   → renderiza junto ao mapa (usa coordenadas de tela)
    AddObject(bgObject);
    AddObject(mapObject);
    AddObject(hudObject);
    AddObject(playerObject);
    AddObject(spawnerObj);
}

StageState::~StageState() {}

void StageState::Start() {
    LoadAssets();
    StartArray();
}

void StageState::LoadAssets() {}

void StageState::Update(float dt) {
    InputManager& input = InputManager::GetInstance();

    if (input.QuitRequested())  quitRequested = true;

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
            Collider* ca = objectArray[i]->GetComponent<Collider>();
            Collider* cb = objectArray[j]->GetComponent<Collider>();
            if (ca && cb) {
                float angA = objectArray[i]->angleDeg * (M_PI / 180.0f);
                float angB = objectArray[j]->angleDeg * (M_PI / 180.0f);
                if (Collision::IsColliding(ca->box, cb->box, angA, angB)) {
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
    // Ordena por box.y: objetos mais acima (y menor) renderizam primeiro (atrás).
    // O bgObject (-10000) sempre fica no fundo; o player (y~430) na frente do mapa.
    std::sort(objectArray.begin(), objectArray.end(),
        [](const std::shared_ptr<GameObject>& a, const std::shared_ptr<GameObject>& b) {
            return a->box.y < b->box.y;
        }
    );
    RenderArray();
}

void StageState::Pause()  {}
void StageState::Resume() {}
