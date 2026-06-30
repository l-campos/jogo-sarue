#define PATROL_RANGE 200.0f
#define PATROL_SPEED 150.0f
#define DIVE_SPEED 400.0f

#include <cmath>
#include "enemy.h"
#include "character.h"
#include "spriterenderer.h"
#include "animator.h"
#include "collider.h"
#include "gameobject.h"


Enemy::Enemy(GameObject& associated, float startX, float startY) 
    : Component(associated), startX(startX), startY(startY), hp(2), isStunned(false) {
    
    // Posiciona o Pombo
    associated.box.x = startX;
    associated.box.y = startY;

    state = PATROL;
    speed = {PATROL_SPEED, 0}; // Começa voando para a direita

    SpriteRenderer* sprite = new SpriteRenderer(associated, "img/Enemy.png", 3, 2);
    associated.AddComponent(sprite);

    Animator* animator = new Animator(associated);
    animator->AddAnimation("idle", Animation(0, 3, 0.1f)); //placeholder voando
    animator->AddAnimation("walking", Animation(4, 1, 0.1f)); //placeholder ataque
    associated.AddComponent(animator);
    
    Collider* collider = new Collider(associated);
    associated.AddComponent(collider);
}

void Enemy::Update(float dt) {
    if (isStunned) {
        damageCooldown.Update(dt);
        
        // FÍSICA DE KNOCKBACK ENQUANTO ESTÁ ATORDOADO
        if (damageCooldown.Get() < 0.3f) {
            speed.y += 1500.0f * dt; 
            associated.box.x += speed.x * dt;
            associated.box.y += speed.y * dt;
            
            Collider* collider = associated.GetComponent<Collider>();
            if (collider != nullptr) collider->Update(dt);
            return; // Sai do Update!
        } else {
            isStunned = false; // Acabou o tempo de stun
            
            // CORREÇÃO: Garante que o pássaro não vai cair no abismo depois da gravidade do knockback
            speed.y = -200.0f; 
        }
    }
    
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
        if (Character::player != nullptr && !Character::player->IsPlayingDead()) {
            Vec2 playerPos = Character::player->GetPosition();
            
            // Calcula a distância X
            float distX = std::abs(associated.box.Center().x - playerPos.x);
            
            // E o Saruê estiver abaixo dele (Y maior)
            if (distX < 100.0f && playerPos.y > associated.box.y) {
                state = DIVE; 
                
                Vec2 direction = playerPos - associated.box.Center();
                speed = direction.Normalize() * DIVE_SPEED;
            }
        }
    } 
    
    else if (state == DIVE) {
        if (animator) animator->SetAnimation("walking"); 
        
        if (Character::player != nullptr) {
            
            // SE O SARUÊ FINGIR DE MORTO DURANTE O VOO:
            if (Character::player->IsPlayingDead()) {
                state = RECOVER; 
                speed.y = -350.0f; 
                speed.x = (speed.x > 0) ? PATROL_SPEED : -PATROL_SPEED;
            } 
            else {
                // Continua teleguiado se o jogador estiver vivo e em pé
                Vec2 playerPos = Character::player->GetPosition();
                Vec2 direction = playerPos - associated.box.Center();
                speed = direction.Normalize() * DIVE_SPEED;

                if (direction.y < 150.0f) {
                    direction.y = 150.0f; 
                }                
                
                if (sprite) {
                    if (speed.x < 0) sprite->SetFlip(SDL_FLIP_HORIZONTAL);
                    else sprite->SetFlip(SDL_FLIP_NONE);
                }
            }
        }

        associated.box.x += speed.x * dt;
        associated.box.y += speed.y * dt;

        // Se passar da altura do chão, sobe (caso o jogador desvie correndo)
        if (associated.box.y > startY + 300.0f) { 
            state = RECOVER;
            speed.y = -200.0f; 
            speed.x = (speed.x > 0) ? PATROL_SPEED : -PATROL_SPEED;
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

    // Se o objeto ficar 500 pixels para trás da câmera, deleta-o para salvar memória!
    if (associated.box.x < Camera::pos.x - 500.0f) {
        associated.RequestDelete();
    }
}

void Enemy::Render() {}
void Enemy::NotifyCollision(GameObject& other) {
    // Por enquanto nada, depois colocamos para dar dano no Saruê!
}

void Enemy::Damage(int damage, Vec2 attackerPos) {
    // CORREÇÃO: Verifica apenas se o pássaro já não está atordoado
    if (!isStunned) { 
        hp -= damage;
        isStunned = true;
        damageCooldown.Restart();

        // O Knockback
        speed.y = -300.0f;
        if (associated.box.Center().x > attackerPos.x) {
            speed.x = 300.0f;
        } else {
            speed.x = -300.0f;
        }
        
        // Força ele a entrar em modo de recuperação para voltar a voar na altura certa depois do golpe
        state = RECOVER; 
        
        if (hp <= 0) associated.RequestDelete();
    }
}