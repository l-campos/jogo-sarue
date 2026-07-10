#include "boss.h"
#include "spriterenderer.h"
#include "animator.h"
#include "collider.h"
#include "character.h"
#include "camera.h"
#include "gamedata.h"
#include "sound.h"
#include "music.h"
#include <cmath>

const float TILE_SIZE = 128.0f;

Boss::Boss(GameObject& associated, float startX, float startY, TileMap* map)
    : Component(associated), state(WAITING), map(map) {
    
    associated.box.x = startX;
    associated.box.y = startY;
    speed = {0, 0};

    // Nasce usando o sprite original do Saruê em escala gigante, sem gritar
    SpriteRenderer* sprite = new SpriteRenderer(associated, "img/sarue.png", 4, 8);
    sprite->SetScale(10.0f, 10.0f);
    sprite->SetFlip(SDL_FLIP_HORIZONTAL); // Fica olhando para a esquerda
    associated.AddComponent(sprite);

    Animator* animator = new Animator(associated);
    animator->AddAnimation("idle", Animation(0, 0, 0.1f));
    animator->AddAnimation("scream", Animation(0, 3, 0.15f)); // Usará os frames do saruebite.png depois
    associated.AddComponent(animator);
}

void Boss::Update(float dt) {
    stateTimer.Update(dt);

    Animator* animator = associated.GetComponent<Animator>();
    SpriteRenderer* sprite = associated.GetComponent<SpriteRenderer>();

    // 1. ESPERANDO O JOGADOR
    if (state == WAITING) {
        if (Character::player != nullptr) {
            Vec2 playerPos = Character::player->GetPosition();
            
            // GATILHO: Jogador no Tile 91 (X) e na parte alta do mapa (Y < 3)
            if (playerPos.x > 91 * TILE_SIZE && playerPos.y < 3 * TILE_SIZE) {
                // Inicia o pulo imediatamente com o sprite normal
                state = JUMPING;
                
                // Rouba a câmera do jogador para focar no Boss
                Camera::Follow(&associated);
                Music silenciador;
                silenciador.Stop(500);

                speed.y = -800.0f; // Impulso forte pra cima
                speed.x = -300.0f; // Pula na direção do jogador
            }
        }
    }
    // 2. NO AR
    else if (state == JUMPING) {
        speed.y += 1500.0f * dt; // Aplica gravidade
        associated.box.x += speed.x * dt;
        associated.box.y += speed.y * dt;

        // Checagem de colisão com o chão
        int bottom = std::floor((associated.box.y + associated.box.h) / TILE_SIZE);
        int centerX = std::floor(associated.box.Center().x / TILE_SIZE);

        if (speed.y > 0 && map != nullptr) {
            if (map->IsWall(centerX, bottom) || map->IsOneWay(centerX, bottom)) {
                associated.box.y = (bottom * TILE_SIZE) - associated.box.h - 0.1f;
                speed.y = 0;
                speed.x = 0;
                state = LANDED;
                stateTimer.Restart();

                // Som de impacto caindo pesado no chão
                Sound("audio/boom.mp3").Play(1);
            }
        }
    }
    // 3. ATERRISSOU (Pausa Dramática)
    else if (state == LANDED) {
        // Fica parado encarando o jogador por meio segundo para dar peso à queda
        if (stateTimer.Get() > 0.5f) {
            state = SCREAMING;
            stateTimer.Restart();

            // A MUTAÇÃO: Troca para o sprite de "Grito" (o mesmo do ataque do jogador)
            if (sprite) {
                sprite->Open("img/saruebite.png");
                sprite->SetFrameCount(4, 1);
                sprite->SetScale(10.0f, 10.0f); 
                sprite->SetFlip(SDL_FLIP_HORIZONTAL); 
            }

            if (animator) animator->SetAnimation("scream");
            
            // Toca um som intimidador do grito
            Sound("audio/roar.mp3").Play(1); 
        }
    }
    // 4. GRITANDO
    else if (state == SCREAMING) {
        // Fica gritando com a boca aberta por 2 segundos antes de encerrar
        if (stateTimer.Get() > 2.0f) {
            GameData::playerVictory = true; 
        }
    }
}

void Boss::Render() {}
void Boss::NotifyCollision(GameObject& other) {}