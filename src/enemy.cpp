#include <cmath>
#include "enemy.h"
#include "character.h"
#include "spriterenderer.h"
#include "animator.h"
#include "collider.h"
#include "gameobject.h"

Enemy::Enemy(GameObject& associated, float startX, float startY) 
    : Component(associated), startX(startX), startY(startY) {
    
    // Posiciona o Pombo
    associated.box.x = startX;
    associated.box.y = startY;

    state = PATROL;
    speed = {PATROL_SPEED, 0}; // Começa voando para a direita

    SpriteRenderer* sprite = new SpriteRenderer(associated, "img/Player.png", 3, 4);
    associated.AddComponent(sprite);

    Animator* animator = new Animator(associated);
    animator->AddAnimation("idle", Animation(0, 5, 0.1f)); //placeholder voando
    animator->AddAnimation("walking", Animation(4, 4, 1.0f)); //placeholder ataque
    associated.AddComponent(animator);
    
    Collider* collider = new Collider(associated);
    associated.AddComponent(collider);
}

void Enemy::Update(float dt) {
    Animator* animator = associated.GetComponent<Animator>();
    SpriteRenderer* sprite = associated.GetComponent<SpriteRenderer>();

    // LÓGICA DE ESTADOS DA INTELIGÊNCIA ARTIFICIAL
    if (state == PATROL) {
        if (animator) animator->SetAnimation("idle");
        
        // Vai de um lado para o outro
        associated.box.x += speed.x * dt;
        
        if (associated.box.x > startX + PATROL_RANGE) {
            speed.x = -PATROL_SPEED; // Vira pra esquerda
            if (sprite) sprite->SetFlip(SDL_FLIP_HORIZONTAL);
        } else if (associated.box.x < startX - PATROL_RANGE) {
            speed.x = PATROL_SPEED; // Vira pra direita
            if (sprite) sprite->SetFlip(SDL_FLIP_NONE);
        }

        // Detecta o Saruê abaixo dele
        if (Character::player != nullptr) {
            // Pede a posição ao Saruê usando GetPosition()
            Vec2 playerPos = Character::player->GetPosition();
            
            // Calcula a distância X
            float distX = std::abs(associated.box.Center().x - playerPos.x);
            
            // E o Saruê estiver abaixo dele (Y maior)
            if (distX < 100.0f && playerPos.y > associated.box.y) {
                state = DIVE; // ATACA!
                
                // Calcula o vetor de direção do rasante
                Vec2 direction = playerPos - associated.box.Center();
                speed = direction.Normalize() * DIVE_SPEED;
            }
        }
    } 
    
    else if (state == DIVE) {
        if (animator) animator->SetAnimation("walking");
        
        // Desce rasgando na diagonal
        associated.box.x += speed.x * dt;
        associated.box.y += speed.y * dt;

        // Se ele passar da altura do chão (ou passar do jogador), começa a subir
        if (associated.box.y > startY + 350.0f) { // Chutando que o chão fica 300 pixels abaixo
            state = RECOVER;
            speed.y = -200.0f; // Sobe
            speed.x = (speed.x > 0) ? PATROL_SPEED : -PATROL_SPEED; // Mantém a inércia horizontal
        }
    } 
    
    else if (state == RECOVER) {
        if (animator) animator->SetAnimation("walking");
        
        // Sobe e anda pra frente
        associated.box.x += speed.x * dt;
        associated.box.y += speed.y * dt;

        // Quando chegar na altura original de voo, volta a patrulhar
        if (associated.box.y <= startY) {
            associated.box.y = startY;
            speed.y = 0;
            state = PATROL;
        }
    }
    
    // Atualiza o colisor
    Collider* collider = associated.GetComponent<Collider>();
    if (collider != nullptr) collider->Update(dt);
}

void Enemy::Render() {}
void Enemy::NotifyCollision(GameObject& other) {
    // Por enquanto nada, depois colocamos para dar dano no Saruê!
}