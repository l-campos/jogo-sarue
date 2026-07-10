#define PATROL_RANGE 200.0f
#define PATROL_SPEED 150.0f
#define DIVE_SPEED 600.0f

#include <cmath>
#include "enemy.h"
#include "character.h"
#include "spriterenderer.h"
#include "animator.h"
#include "collider.h"
#include "gameobject.h"

const float TILE_SIZE = 128.0f;

Enemy::Enemy(GameObject& associated, float startX, float startY, TileMap* map) 
    : Component(associated), startX(startX), startY(startY), hp(2), isStunned(false), map(map) {
    
    associated.box.x = startX;
    associated.box.y = startY;

    state = PATROL;
    speed = {PATROL_SPEED, 0}; 

    // CARREGA APENAS A IMAGEM DE PATRULHA (4 frames)
    SpriteRenderer* sprite = new SpriteRenderer(associated, "img/pombo_patrulha.png", 4, 1);
    sprite->SetScale(2.0f, 2.0f);
    associated.AddComponent(sprite);

    Animator* animator = new Animator(associated);
    
    // Animações começam do 0 em seus respectivos arquivos
    animator->AddAnimation("patrol", Animation(0, 3, 0.1f)); 
    animator->AddAnimation("diving", Animation(0, 3, 0.1f)); 
    animator->AddAnimation("explode", Animation(4, 4, 0.2f)); 
    
    associated.AddComponent(animator);
    
    Collider* collider = new Collider(associated);
    associated.AddComponent(collider);
}

void Enemy::Update(float dt) {
    if (isStunned) {
        damageCooldown.Update(dt);
        
        if (damageCooldown.Get() < 0.3f) {
            speed.y += 1500.0f * dt; 
            associated.box.x += speed.x * dt;
            associated.box.y += speed.y * dt;
            
            Collider* collider = associated.GetComponent<Collider>();
            if (collider != nullptr) collider->Update(dt);
            return; 
        } else {
            isStunned = false; 
            speed.y = -200.0f; 
        }
    }
    
    Animator* animator = associated.GetComponent<Animator>();
    SpriteRenderer* sprite = associated.GetComponent<SpriteRenderer>();

    // VERIFICA SE O SARUÊ FINGIU DE MORTO
    if (Character::player != nullptr) {
        // Se Saruê deitou, o Pombo deve abortar (caso não tenha explodido ainda)
        if (Character::player->IsPlayingDead() && state != EXPLODE && state != RETURNING) {
            // Se ele já estava em DIVE (mergulhando), precisamos destrocar a imagem de volta para patrulha!
            if (state == DIVE) {
                if (sprite) {
                    sprite->Open("img/pombo_patrulha.png");
                    sprite->SetFrameCount(4, 1);
                    sprite->SetScale(2.0f, 2.0f);
                }
            }
            state = RETURNING;
        } 
        // Se o Saruê levantou e o pombo estava no ar voltando, volta a patrulhar imediatamente
        else if (!Character::player->IsPlayingDead() && state == RETURNING) {
            state = PATROL;
        }
    }

    if (state == PATROL) {
        if (animator) animator->SetAnimation("patrol");
        
        associated.box.x += speed.x * dt;
        
        // Vai e volta na patrulha
        if (associated.box.x > startX + PATROL_RANGE) {
            speed.x = -PATROL_SPEED; 
            if (sprite) sprite->SetFlip(SDL_FLIP_HORIZONTAL);
        } else if (associated.box.x < startX - PATROL_RANGE) {
            speed.x = PATROL_SPEED; 
            if (sprite) sprite->SetFlip(SDL_FLIP_NONE);
        }

        // Procura o jogador
        if (Character::player != nullptr && !Character::player->IsPlayingDead()) {
            Vec2 playerPos = Character::player->GetPosition();
            float distX = std::abs(associated.box.Center().x - playerPos.x);
            
            // Viu o jogador = Mudar para DIVE
            if (distX < 200.0f && playerPos.y > associated.box.y) {
                diveTarget = playerPos;
                state = DIVE; 

                // TROCA PARA A IMAGEM DE MERGULHO EXATAMENTE AQUI (Apenas 1 vez)
                if (sprite) {
                    sprite->Open("img/pombo_mergulho.png");
                    sprite->SetFrameCount(5, 1);
                    sprite->SetScale(2.0f, 2.0f);
                    sprite->SetFlip(speed.x < 0 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
                }
                
                Vec2 direction = (diveTarget - associated.box.Center()).Normalize();
                speed = direction * DIVE_SPEED;
            }
        }
    } 
    
    else if (state == DIVE) {
        if (animator) animator->SetAnimation("diving"); 

        associated.box.x += speed.x * dt;
        associated.box.y += speed.y * dt;

        Vec2 currentPos = associated.box.Center();
        float distToTarget = currentPos.Distance(diveTarget);
        bool hitGround = false;

        // Checagem com o chão (TileMap)
        if (map != nullptr && speed.y > 0) {
            int left = std::floor(associated.box.x / TILE_SIZE);
            int right = std::floor((associated.box.x + associated.box.w - 1.0f) / TILE_SIZE);
            int bottom = std::floor((associated.box.y + associated.box.h) / TILE_SIZE);

            for (int x = left; x <= right; ++x) {
                if (map->IsWall(x, bottom) || map->IsOneWay(x, bottom)) {
                    hitGround = true; 
                    break;
                }
            }
        }

        bool reachedTarget = distToTarget < 24.0f;
        if (reachedTarget || hitGround) {
            state = EXPLODE;
            explodeTimer.Restart();
            speed = Vec2(0, 0);
        }
    }

    else if (state == RETURNING) {
        if (animator) animator->SetAnimation("patrol");

        Vec2 currentPos = associated.box.Center();
        // O alvo de retorno é a base dele
        Vec2 homePos(startX + (associated.box.w / 2.0f), startY + (associated.box.h / 2.0f));
        float distToHome = currentPos.Distance(homePos);

        if (distToHome < 10.0f) {
            // Chegou em casa
            associated.box.x = startX;
            associated.box.y = startY;
            speed = {PATROL_SPEED, 0};
            state = PATROL;
        } else {
            // Voa na direção de casa
            Vec2 direction = (homePos - currentPos).Normalize();
            speed = direction * PATROL_SPEED;
            
            associated.box.x += speed.x * dt;
            associated.box.y += speed.y * dt;

            // Vira o sprite para o lado do voo
            if (sprite) {
                sprite->SetFlip(speed.x < 0 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
            }
        }
    }
    
    else if (state == EXPLODE) {
        if (animator) animator->SetAnimation("explode");
        sprite->SetScale(3.0f, 3.0f);
        
        Collider* collider = associated.GetComponent<Collider>();
        if (collider) {
            collider->Update(dt);
        }

        explodeTimer.Update(dt);
        if (explodeTimer.Get() >= 0.80f) {
            associated.RequestDelete();
        }
        return;
    }
    
    Collider* collider = associated.GetComponent<Collider>();
    if (collider != nullptr) collider->Update(dt);

    if (associated.box.x < Camera::pos.x - 500.0f) {
        associated.RequestDelete();
    }
}

void Enemy::Render() {}
void Enemy::NotifyCollision(GameObject& other) {}

void Enemy::Damage(int damage, Vec2 attackerPos) {
    if (!isStunned && state != EXPLODE) { 
        hp -= damage;
        isStunned = true;
        damageCooldown.Restart();

        speed.y = -300.0f;
        if (associated.box.Center().x > attackerPos.x) {
            speed.x = 300.0f;
        } else {
            speed.x = -300.0f;
        }
        
        if (hp <= 0) associated.RequestDelete();
    }
}