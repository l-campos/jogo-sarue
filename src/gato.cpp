#include <cmath>
#include "gato.h"
#include "character.h"
#include "spriterenderer.h"
#include "animator.h"
#include "collider.h"
#include "game.h"
#include "fruit.h"
#include "state.h"

const float GRAVITY = 1500.0f;
const float JUMP_FORCE = 700.0f;
const float PATROL_SPEED = 100.0f;
const float CHASE_SPEED = 300.0f;
const float JUMP_RANGE = 400.0f;
const float AGGRO_RANGE = 500.0f;

// Tamanhos fixos ignorados, vamos setar o FrameCount dinamicamente
const float TILE_SIZE = 128.0f;

Gato::Gato(GameObject& associated, float x, float y, TileMap* map)
    : Component(associated), state(PATROL), hp(2), startX(x), isGrounded(false), isStunned(false), map(map) {
    
    associated.box.x = x;
    associated.box.y = y;
    speed = {PATROL_SPEED, 0};
    
    // CARREGA APENAS A IMAGEM DE CORRIDA (4 frames)
    SpriteRenderer* sprite = new SpriteRenderer(associated, "img/gato_correndo.png", 4, 1);
    sprite->SetScale(2.0f, 2.0f);
    associated.AddComponent(sprite);

    Animator* animator = new Animator(associated);

    animator->AddAnimation("running", Animation(0, 3, 0.1f));
    animator->AddAnimation("attack_jump", Animation(0, 5, 0.1f));

    associated.AddComponent(animator);

    Collider* collider = new Collider(associated);
    associated.AddComponent(collider);
}

void Gato::Update(float dt) {
    if (isStunned) {
        damageCooldown.Update(dt);
        if (damageCooldown.Get() >= 0.3f) {
            isStunned = false; 
        }
    }

    Animator* animator = associated.GetComponent<Animator>();
    SpriteRenderer* sprite = associated.GetComponent<SpriteRenderer>();
    Collider* collider = associated.GetComponent<Collider>();

    speed.y += GRAVITY * dt;
    associated.box.x += speed.x * dt;

    isGrounded = false;
    if (map != nullptr) {
        int left = std::floor(associated.box.x / TILE_SIZE);
        int right = std::floor((associated.box.x + associated.box.w - 1.0f) / TILE_SIZE);

        bool hitWall = false;
        int checkY = std::floor((associated.box.y + (associated.box.h/2)) / TILE_SIZE);
        if (speed.x > 0 && map->IsWall(right, checkY)) hitWall = true;
        else if (speed.x < 0 && map->IsWall(left, checkY)) hitWall = true;

        if (hitWall) {
            speed.x *= -1; 
            if (sprite) sprite->SetFlip(speed.x < 0 ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL);
            startX = associated.box.x; 
        }

        associated.box.y += speed.y * dt;
        int bottom = std::floor((associated.box.y + associated.box.h - 1.0f) / TILE_SIZE);

        if (speed.y > 0) {
            bool hitFloor = false;
            for (int x = left; x <= right; ++x) {
                if (map->IsWall(x, bottom) || map->IsOneWay(x, bottom)) {
                    hitFloor = true; break;
                }
            }
            if (hitFloor) {
                associated.box.y = (bottom * TILE_SIZE) - associated.box.h - 0.1f;
                speed.y = 0;
                isGrounded = true;
            }
        }
    } else {
        associated.box.y += speed.y * dt;
        if (associated.box.y + associated.box.h >= 1020.0f) {
            associated.box.y = 1020.0f - associated.box.h;
            speed.y = 0;
            isGrounded = true;
        }
    }

    if (!isStunned) {
        if (state == PATROL) {
            if (animator) animator->SetAnimation("running");

            if (associated.box.x > startX + 150.0f) {
                speed.x = -PATROL_SPEED;
                if (sprite) sprite->SetFlip(SDL_FLIP_NONE);
            } else if (associated.box.x < startX - 150.0f) {
                speed.x = PATROL_SPEED;
                if (sprite) sprite->SetFlip(SDL_FLIP_HORIZONTAL);
            }

            if (Character::player != nullptr) {
                float distX = std::abs(associated.box.Center().x - Character::player->GetPosition().x);
                float distY = std::abs(associated.box.Center().y - Character::player->GetPosition().y);

                // NOVA REGRA: Só ataca se estiver na mesma linha horizontal (Tolerância de 64 pixels)
                if (distX < AGGRO_RANGE && distY < 64.0f) state = CHASE;
            }
        }
        
        else if (state == CHASE) {
            if (animator) animator->SetAnimation("running");

            if (Character::player != nullptr) {
                float playerX = Character::player->GetPosition().x;
                float distX = std::abs(associated.box.Center().x - playerX);

                speed.x = (playerX > associated.box.Center().x) ? CHASE_SPEED : -CHASE_SPEED;
                
                if (sprite) {
                    if (speed.x < 0) sprite->SetFlip(SDL_FLIP_NONE);
                    else sprite->SetFlip(SDL_FLIP_HORIZONTAL);
                }

                if (distX < JUMP_RANGE && isGrounded) {
                    speed.y = -JUMP_FORCE; 
                    isGrounded = false;
                    state = JUMP;

                    // TROCA PARA A IMAGEM DE ATAQUE (6 frames)
                    if (sprite) {
                        sprite->Open("img/gato_atacando.png");
                        sprite->SetFrameCount(6, 1);
                        sprite->SetScale(2.0f, 2.0f); // Força atualização do tamanho da box
                    }
                }
            } 
        }
        
        else if (state == JUMP) {
            if (animator) animator->SetAnimation("attack_jump");

            if (speed.y > -100.0f) { 
                state = ATTACK;
                if (collider) collider->SetScale({1.5f, 1.0f}); 
            }
        }
        
        else if (state == ATTACK) {
            if (animator) animator->SetAnimation("attack_jump");
            
            if (isGrounded) {
                state = COOLDOWN;
                speed.x = 0; 
                cooldownTimer.Restart();
                
                if (collider) collider->SetScale({1.0f, 1.0f});
            }
        }
        
        else if (state == COOLDOWN) {
            // Congela na animação final do ataque
            cooldownTimer.Update(dt);
            if (cooldownTimer.Get() > 1.0f) { 
                state = CHASE; 

                // DEVOLVE PARA A IMAGEM DE CORRIDA
                if (sprite) {
                    sprite->Open("img/gato_correndo.png");
                    sprite->SetFrameCount(4, 1);
                    sprite->SetScale(2.0f, 2.0f); // Força atualização do tamanho da box
                }
            }
        }
    }

    if (collider != nullptr) collider->Update(dt);

    if (associated.box.x < Camera::pos.x - 500.0f) {
        associated.RequestDelete();
    }
}

void Gato::Render() {}
void Gato::NotifyCollision(GameObject& other) {}

void Gato::Damage(int damage, Vec2 attackerPos) {
    if (!isStunned) { 
        hp -= damage;
        isStunned = true;
        damageCooldown.Restart();

        speed.y = -300.0f;
        if (associated.box.Center().x > attackerPos.x) {
            speed.x = 400.0f;
        } else {
            speed.x = -400.0f;
        }
        isGrounded = false;

        if (hp <= 0) {
            GameObject* fruitObj = new GameObject();
            Fruit* fruit = new Fruit(*fruitObj, associated.box.Center().x, associated.box.Center().y);
            fruitObj->AddComponent(fruit);
            Game::GetInstance().GetCurrentState().AddObject(fruitObj);

            associated.RequestDelete();
        }
    }
}