#include <cmath>
#include "gato.h"
#include "character.h"
#include "spriterenderer.h"
#include "animator.h"
#include "collider.h"
#include "bullet.h"

const float GRAVITY = 1500.0f;
const float JUMP_FORCE = 500.0f;   // Força do pulo
const float PATROL_SPEED = 100.0f; // Andando calmo
const float CHASE_SPEED = 300.0f;  // Correndo atrás do Saruê
const float JUMP_RANGE = 200.0f;   // Distância para dar o bote
const float AGGRO_RANGE = 500.0f;  // Visão do gato

Gato::Gato(GameObject& associated, float x, float y)
    : Component(associated), state(PATROL), hp(2), startX(x), isGrounded(false), groundLevel(700.0f) {
    associated.box.x = x;
    associated.box.y = y;
    speed = {PATROL_SPEED, 0};
    
    SpriteRenderer* sprite = new SpriteRenderer(associated, "img/NPC.png", 3, 4);
    associated.AddComponent(sprite);

    Animator* animator = new Animator(associated);
    animator->AddAnimation("patrol", Animation(6, 9, 0.15f));
    animator->AddAnimation("attack", Animation(0, 5, 0.15f));
    associated.AddComponent(animator);

    Collider* collider = new Collider(associated);
    associated.AddComponent(collider);
}

void Gato::Update(float dt) {
    Animator* animator = associated.GetComponent<Animator>();
    SpriteRenderer* sprite = associated.GetComponent<SpriteRenderer>();
    Collider* collider = associated.GetComponent<Collider>();

    speed.y += GRAVITY * dt;
    associated.box.x += speed.x * dt;
    associated.box.y += speed.y * dt;

    isGrounded = false;
    if (associated.box.y + associated.box.h >= groundLevel) {
        associated.box.y = groundLevel - associated.box.h;
        speed.y = 0;
        isGrounded = true;
    }

    if (state == PATROL) {
        if (animator) animator->SetAnimation("patrol");

        // Vai e volta na área dele
        if (associated.box.x > startX + 150.0f) {
            speed.x = -PATROL_SPEED;
            if (sprite) sprite->SetFlip(SDL_FLIP_HORIZONTAL);
        } else if (associated.box.x < startX - 150.0f) {
            speed.x = PATROL_SPEED;
            if (sprite) sprite->SetFlip(SDL_FLIP_NONE);
        }

        // Se o Saruê chegar perto e não estiver fingindo de morto:
        if (Character::player != nullptr) {
            float distX = std::abs(associated.box.Center().x - Character::player->GetPosition().x);
            if (distX < AGGRO_RANGE) state = CHASE;
        }
    }
    
    else if (state == CHASE) {
        if (animator) animator->SetAnimation("attack");

        if (Character::player != nullptr) {
            float playerX = Character::player->GetPosition().x;
            float distX = std::abs(associated.box.Center().x - playerX);

            // Acelera na direção do jogador
            speed.x = (playerX > associated.box.Center().x) ? CHASE_SPEED : -CHASE_SPEED;
            
            if (sprite) {
                if (speed.x < 0) sprite->SetFlip(SDL_FLIP_HORIZONTAL);
                else sprite->SetFlip(SDL_FLIP_NONE);
            }

            // Se chegou perto o suficiente, dá o BOTE!
            if (distX < JUMP_RANGE && isGrounded) {
                speed.y = -JUMP_FORCE; 
                isGrounded = false;
                state = JUMP;
            }
        } 
    }
    
    else if (state == JUMP) {
        if (animator) animator->SetAnimation("attack");

        // Assim que ele atinge o pico do pulo e começa a cair, ele ativa o ataque
        if (speed.y > -100.0f) { 
            state = ATTACK;
            // ESTICA O COLISOR PARA SIMULAR A GARRA (Aumenta a largura em 50%)
            if (collider) collider->SetScale({1.5f, 1.0f}); 
        }
    }
    
    else if (state == ATTACK) {
        if (animator) animator->SetAnimation("attack");
        // No ar, ele mantém a inércia do pulo para frente até bater no chão
        
        if (isGrounded) {
            state = COOLDOWN;
            speed.x = 0; // Trava no chão para respirar
            cooldownTimer.Restart();
            
            // VOLTA O COLISOR AO NORMAL (Retrai a garra)
            if (collider) collider->SetScale({1.0f, 1.0f});
        }
    }
    
    else if (state == COOLDOWN) {
        if (animator) animator->SetAnimation("patrol");

        cooldownTimer.Update(dt);
        if (cooldownTimer.Get() > 1.0f) { // Fica 1 segundo parado no chão
            state = CHASE; // Volta a correr implacavelmente!
        }
    }

    // 4. ATUALIZA COLISOR
    if (collider != nullptr) collider->Update(dt);
}

void Gato::Render() {}

void Gato::NotifyCollision(GameObject& other) {
    if (other.GetComponent<Bullet>() != nullptr) {
        hp--;
        other.RequestDelete();
        if (hp < 0) {
            associated.RequestDelete();
        }
    }
}