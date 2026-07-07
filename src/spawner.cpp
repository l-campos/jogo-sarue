#include <cstdlib>  
#include "spawner.h"
#include "game.h"
#include "camera.h"
#include "enemy.h"  
#include "gato.h"
#include "fruit.h"
#include "stagestate.h"

Spawner::Spawner(GameObject& associated) : Component(associated) {
    pomboCooldown = 2.0f;
    gatoCooldown = 4.0f;
    frutaCooldown = 8.0f;
}

void Spawner::Update(float dt) {
    pomboTimer.Update(dt);
    gatoTimer.Update(dt);
    frutaTimer.Update(dt);

    State& state = Game::GetInstance().GetCurrentState();

    // Posição X: Nasce sempre à direita da tela (ex: 1200 pixels à frente da câmera)
    float spawnX = Camera::pos.x + 100.0f; 

    // 1. SPAWN DO POMBO
    if (pomboTimer.Get() > pomboCooldown) {
        pomboTimer.Restart();
        // Próximo pombo vem entre 2 a 5 segundos
        pomboCooldown = 2.0f + (rand() % 4); 

        GameObject* pomboObj = new GameObject();
        /*POMBO SURGINDO*/
        float spawnY = 10.0f + (rand() % 150);
        Enemy* pombo = new Enemy(*pomboObj, spawnX, spawnY);
        pomboObj->AddComponent(pombo);
        state.AddObject(pomboObj);
    }

    // 2. SPAWN DO GATO
    if (gatoTimer.Get() > gatoCooldown) {
        gatoTimer.Restart();
        gatoCooldown = 4.0f + (rand() % 5); 

        GameObject* gatoObj = new GameObject();
        /*GATO SURGINDO*/
        Gato* gato = new Gato(*gatoObj, spawnX, 450.0f); 
        gatoObj->AddComponent(gato);
        state.AddObject(gatoObj);
    }

    // 3. SPAWN DA Fruit
    if (frutaTimer.Get() > frutaCooldown) {
        frutaTimer.Restart();
        frutaCooldown = 10.0f + (rand() % 6); 
        /*FRUTA SURGINDO*/
        GameObject* fruitObj = new GameObject();
        Fruit* fruit = new Fruit(*fruitObj, spawnX, 450.0f);
        fruitObj->AddComponent(fruit);
        state.AddObject(fruitObj);
    }
}

void Spawner::Render() {}