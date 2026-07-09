#include "character.h"
#include "spriterenderer.h"
#include "game.h"
#include "gameobject.h"
#include "animator.h"
#include "stagestate.h"
#include "collider.h"
#include "enemy.h"
#include "sound.h"
#include "fruit.h"
#include "gato.h"
#include "fumaca.h"
#include "meleeattack.h"
#include <cmath>

const float GRAVITY = 1500.0f;
const float JUMP_FORCE = 700.0f;
const float MOVE_SPEED = 300.0f;
const float MAX_FALL_SPEED = 800.0f;
const float DASH_SPEED = 1000.0f;
const float DASH_TIME = 0.2f;     
const float DASH_COOLDOWN_TIME = 0.5f; 

const float TILE_SIZE = 128.0f; 

const float CLIMB_SPEED = 150.0f; 
const float CLIMB_EXIT_JUMP = 500.0f; 
const float ATTACK_ANIM_DURATION = 0.35f; 
const float DEATH_BUMP_SPEED = -350.0f; 

const int SARUE_COLS = 4;
const int SARUE_ROWS = 8;
const int SARUE_FRAME_SIZE = 32;
inline int SF(int row, int col) { return row * SARUE_COLS + col; }

Character* Character::player = nullptr;

Character::Character(GameObject& associated, std::string sprite, TileMap* map)
    : Component(associated), hp(6), linearSpeed(300.0f), isGrounded(false),
      isPlayingDead(false), isDashing(false), isFacingLeft(false), isAttacking(false), 
      isScaling(false), isHanging(false), isDying(false), droppingDown(false), map(map) {
    
    player = this;

    SpriteRenderer* jogador = new SpriteRenderer(associated, sprite, SARUE_COLS, SARUE_ROWS);
    jogador->SetFrameSize(SARUE_FRAME_SIZE, SARUE_FRAME_SIZE);
    jogador->SetScale(4.0f, 4.0f);
    associated.AddComponent(jogador);

    Animator* animator = new Animator(associated);
    animator->AddAnimation("idle", Animation(SF(0, 0), SF(0, 0), 0.2f));
    animator->AddAnimation("walking", Animation(SF(1, 0), SF(1, 3), 0.1f));
    animator->AddAnimation("jump", Animation(SF(2, 0), SF(2, 3), 0.12f));
    animator->AddAnimation("escala", Animation(SF(3, 0), SF(3, 3), 0.15f));
    animator->AddAnimation("hit", Animation(SF(4, 0), SF(4, 3), 0.1f));
    animator->AddAnimation("dead", Animation(SF(5, 0), SF(5, 3), 0.15f));
    animator->AddAnimation("play", Animation(SF(6, 3), SF(6, 3), 0.25f));
    animator->AddAnimation("pendura", Animation(SF(7, 0), SF(7, 0), 0.15f));
    associated.AddComponent(animator);
    animator->SetAnimation("idle");
    
    // ENCAIXE DO PEZINHO: O colisor agora sobe 12 pixels, fazendo o desenho afundar 12 no chão!
    Collider* collider = new Collider(associated, Vec2(1.0f, 1.0f), Vec2(0, -6.0f));
    associated.AddComponent(collider);
}

Character::~Character() {
    if (player == this) {
        player = nullptr;
    }
}

void Character::Start() {
    State& state = Game::GetInstance().GetCurrentState();
    std::weak_ptr<GameObject> character = state.GetObjectPtr(&associated);
}

void Character::Update(float dt) {
    damageCooldown.Update(dt);
    dashTimer.Update(dt);
    dashCooldown.Update(dt);
    attackTimer.Update(dt);

    if (dashTimer.Get() > DASH_TIME) {
        isDashing = false;
    }
    
    bool canControl = damageCooldown.Get() >= 0.3f;
    if (canControl) speed.x = 0;

    if (isAttacking && attackTimer.Get() >= ATTACK_ANIM_DURATION) {
        isAttacking = false;
    }

    SpriteRenderer* spriteRenderer = associated.GetComponent<SpriteRenderer>();
    Animator* animator = associated.GetComponent<Animator>();
    
    isPlayingDead = false;

    if (hp <= 0) {
        if (!isDying) {
            isDying = true;
            speed.x = 0;
            speed.y = DEATH_BUMP_SPEED;
        }
        deathTimer.Update(dt);
        if (animator) animator->SetAnimation("dead");
        
        speed.y += GRAVITY * dt;
        associated.box.x += speed.x * dt;
        associated.box.y += speed.y * dt;

        if (deathTimer.Get() >= 1.5f || associated.box.y > 1200.0f) {
            associated.RequestDelete();
        }
        return; 
    }
    
    float moveIntentY = 0.0f;

    while (!taskQueue.empty()) {
        Command cmd = taskQueue.front();
        taskQueue.pop();
        
        if (!canControl) continue; 
        
        if (cmd.type == Command::PLAY_DEAD){
            if (isGrounded) {
                isPlayingDead = true;
                speed.x = 0;
            }
        }
        else if (cmd.type == Command::DROP_DOWN) {
            if (isGrounded) {
                droppingDown = true;
                dropTimer.Restart();
                isGrounded = false; 
            }
        }
        else if (cmd.type == Command::MOVE) {
            moveIntentY = cmd.pos.y; 
            
            if (isScaling) {
                speed.x = 0;
                speed.y = cmd.pos.y * CLIMB_SPEED;
            } else {
                speed.x = cmd.pos.x * linearSpeed;
                if (speed.x < 0) {
                    spriteRenderer->SetFlip(SDL_FLIP_HORIZONTAL);
                    isFacingLeft = true;
                }
                else if (speed.x > 0) {
                    spriteRenderer->SetFlip(SDL_FLIP_NONE);   
                    isFacingLeft = false;
                }
            }
        } 
        else if (cmd.type == Command::JUMP) {
            if (isScaling) {
                isScaling = false;
                speed.y = -CLIMB_EXIT_JUMP; 
                isGrounded = false;
            }
            else if (isGrounded) {
                speed.y = -JUMP_FORCE;
                isGrounded = false;
            }
        }
        else if (cmd.type == Command::ATTACK) {
            float passDirX = cmd.pos.x;
            if (passDirX == 0) passDirX = isFacingLeft ? -1.0f : 1.0f;

            isAttacking = true;
            attackTimer.Restart();

            GameObject* attackObj = new GameObject();
            MeleeAttack* melee = new MeleeAttack(*attackObj, Game::GetInstance().GetCurrentState().GetObjectPtr(&associated), passDirX, cmd.pos.y);
            attackObj->AddComponent(melee);
            Game::GetInstance().GetCurrentState().AddObject(attackObj);
        }
        else if (cmd.type == Command::DASH && !isPlayingDead) {
            if(!isDashing && dashCooldown.Get() > DASH_COOLDOWN_TIME){
                isDashing = true;
                isScaling = false; 
                dashTimer.Restart();
                dashCooldown.Restart();
            }
        }
    }

    // -------------------------------------------------------------------------
    // LÓGICA DO CIPÓ / ESCALADA
    // -------------------------------------------------------------------------
    if (map != nullptr && !isDashing) {
        Vec2 center = associated.box.Center();
        int tileCenterX = std::floor(center.x / TILE_SIZE);
        int tileCenterY = std::floor(center.y / TILE_SIZE);
        
        bool onClimbTile = map->IsClimbing(tileCenterX, tileCenterY);
        
        // ENTRAR NA ESCALADA
        if (!isScaling && onClimbTile && moveIntentY != 0 && !isPlayingDead) {
            isScaling = true;
            isGrounded = false;
            speed = Vec2(0, 0); 
            
            isFacingLeft = false;
            if (spriteRenderer != nullptr) {
                spriteRenderer->SetFlip(SDL_FLIP_NONE);
            }
            
            float tileWorldX = tileCenterX * TILE_SIZE;
            associated.box.x = (tileWorldX + (TILE_SIZE / 2.0f)) - (associated.box.w / 2.0f);
        }
        // SAIR DA ESCALADA (Acabou o cano pelo topo)
        else if (isScaling && !onClimbTile) {
            isScaling = false; 
            if (speed.y < 0) {
                speed.y = -500.0f; 
            }
        }
        
        // SAIR DA ESCALADA POR BAIXO (Encostou o pé na parede ou chão)
        if (isScaling && speed.y > 0) {
            Collider* col = associated.GetComponent<Collider>();
            Rect hBox = (col != nullptr) ? col->box : associated.box;
            
            float tW = TILE_SIZE;
            float tH = TILE_SIZE;
            int left = std::floor(hBox.x / tW);
            int right = std::floor((hBox.x + hBox.w - 1.0f) / tW);
            
            // Olha para onde o pé vai estar no próximo frame de simulação
            float futureFootY = hBox.y + (speed.y * dt) + hBox.h;
            int bottomTile = std::floor((futureFootY - 1.0f) / tH);
            
            bool hitFloor = false;
            for (int x = left; x <= right; ++x) {
                if (map->IsClimbing(x, bottomTile)) {
                    hitFloor = false; 
                    break; 
                }

                if (map->IsWall(x, bottomTile) || map->IsOneWay(x, bottomTile)) {
                    hitFloor = true; break;
                }
            }
            
            if (hitFloor) {
                isScaling = false; // Solta do cano e a colisão "Y" normal embaixo já toma o controle no mesmo frame!
            }
        }
    }

    if (isDashing) {
        speed.x = isFacingLeft ? -DASH_SPEED : DASH_SPEED; 
        speed.y = 0; 
    }

    if (!isScaling && !isDashing) {
        speed.y += GRAVITY * dt;
        if (speed.y > MAX_FALL_SPEED) speed.y = MAX_FALL_SPEED;
    }

    Collider* collider = associated.GetComponent<Collider>();
    Rect hitbox = (collider != nullptr) ? collider->box : associated.box;

    associated.box.x += speed.x * dt;
    hitbox.x += speed.x * dt; 

    if (map != nullptr && !isScaling) {
        float tW = TILE_SIZE;
        float tH = TILE_SIZE;
        
        int left = std::floor(hitbox.x / tW);
        int right = std::floor((hitbox.x + hitbox.w - 1.0f) / tW);
        int top = std::floor(hitbox.y / tH);
        int bottom = std::floor((hitbox.y + hitbox.h - 1.0f) / tH);

        bool collisionX = false;
        if (speed.x > 0) { 
            for (int y = top; y <= bottom; ++y) {
                if (map->IsWall(right, y)) { collisionX = true; break; } 
            }
            if (collisionX) {
                float diferencaHitbox = hitbox.x - associated.box.x;
                associated.box.x = (right * tW) - hitbox.w - diferencaHitbox - 0.1f;
                hitbox.x = associated.box.x + diferencaHitbox; 
            }
        } else if (speed.x < 0) { 
            for (int y = top; y <= bottom; ++y) {
                if (map->IsWall(left, y)) { collisionX = true; break; } 
            }
            if (collisionX) {
                float diferencaHitbox = hitbox.x - associated.box.x;
                associated.box.x = (left * tW) + tW - diferencaHitbox + 0.1f;
                hitbox.x = associated.box.x + diferencaHitbox; 
            }
        }
    }

    associated.box.y += speed.y * dt;
    hitbox.y += speed.y * dt;
    isGrounded = false;

    dropTimer.Update(dt);
    if (dropTimer.Get() > 0.25f) { 
        droppingDown = false;
    }

    if (map != nullptr && !isScaling) {
        float tW = TILE_SIZE;
        float tH = TILE_SIZE;
        
        int left = std::floor(hitbox.x / tW);
        int right = std::floor((hitbox.x + hitbox.w - 1.0f) / tW);
        int top = std::floor(hitbox.y / tH);
        int bottom = std::floor((hitbox.y + hitbox.h - 1.0f) / tH);

        if (speed.y > 0) { 
            bool collisionWall = false;
            bool collisionOneWay = false;

            for (int x = left; x <= right; ++x) {
                if (map->IsWall(x, bottom)) { collisionWall = true; break; }
                if (!droppingDown && map->IsOneWay(x, bottom)) { collisionOneWay = true; }
            }

            if (collisionWall) {
                float diferencaHitbox = hitbox.y - associated.box.y;
                associated.box.y = (bottom * tH) - hitbox.h - diferencaHitbox - 0.1f;
                hitbox.y = associated.box.y + diferencaHitbox; 
                speed.y = 0;
                isGrounded = true;
            } 
            else if (collisionOneWay) {
                float oldBottom = hitbox.y - (speed.y * dt) + hitbox.h;
                float tileTop = bottom * tH;

                if (oldBottom <= tileTop + 15.0f) { 
                    float diferencaHitbox = hitbox.y - associated.box.y;
                    associated.box.y = tileTop - hitbox.h - diferencaHitbox - 0.1f;
                    hitbox.y = associated.box.y + diferencaHitbox; 
                    speed.y = 0;
                    isGrounded = true;
                }
            }
        } 
        else if (speed.y < 0) { 
            bool collisionWall = false;
            
            for (int x = left; x <= right; ++x) {
                if (map->IsWall(x, top)) { collisionWall = true; break; }
            }

            if (collisionWall) {
                float diferencaHitbox = hitbox.y - associated.box.y;
                associated.box.y = (top * tH) + tH - diferencaHitbox + 0.1f;
                hitbox.y = associated.box.y + diferencaHitbox; 
                speed.y = 0;
            }
        }
    }

    if (hp > 0 && map != nullptr && !isScaling) {
        int tileCenterX = std::floor(hitbox.Center().x / TILE_SIZE);
        int tileCenterY = std::floor(hitbox.Center().y / TILE_SIZE);
        if (map->IsWater(tileCenterX, tileCenterY)) {
            hp = 0;
        }
    }

    if (associated.box.y > 10000.0f) { 
        hp = 0; 
    }

    bool isMoving = (speed.x != 0);
    bool justHit = damageCooldown.Get() < 0.3f;

    if (animator) {
        animator->isPlaying = true; // Garante que a animação despause caso ele volte a andar ou pular

        if (justHit) {
            animator->SetAnimation("hit");
        }
        else if (isPlayingDead) {
            animator->SetAnimation("play");
        }
        else if (isHanging) {
            animator->SetAnimation("pendura");
        }
        else if (isScaling) {
            animator->SetAnimation("escala");
            // PAUSA A ANIMAÇÃO QUANDO ESTIVER PARADO NO CANO
            if (speed.y == 0) {
                animator->isPlaying = false; 
            }
        }
        else if (!isGrounded) {
            animator->SetAnimation("jump");
        }
        else if (isMoving && isGrounded) {
            animator->SetAnimation("walking");
        } 
        else {
            animator->SetAnimation("idle");
        }
    }

    if (collider != nullptr) {
        collider->Update(dt);
    }
}

void Character::Issue(Command task){
    taskQueue.push(task);
}

void Character::NotifyCollision(GameObject& other) {
    if (hp <= 0) return;

    SpriteRenderer* spriteRenderer = associated.GetComponent<SpriteRenderer>();
     
    if (other.GetComponent<Enemy>() != nullptr || other.GetComponent<Gato>() != nullptr) {
        if (damageCooldown.Get() > 1.0f) {
            hp--;
            damageCooldown.Restart();

            if (other.box.Center().x > associated.box.Center().x) {
                speed.x = -400.0f;
                if (spriteRenderer != nullptr) spriteRenderer->SetFlip(SDL_FLIP_NONE);
            }
            else {
                speed.x = 400.0f;
                if (spriteRenderer != nullptr) spriteRenderer->SetFlip(SDL_FLIP_HORIZONTAL);
            }

            speed.y = -300.0f;
            isGrounded = false;
        }
    }

    if (other.GetComponent<Fruit>() != nullptr) {
        hp += 1;
        if (hp > 6) hp = 6; // O máximo de vidas do personagem é 6
        other.RequestDelete();
    }
}

Vec2 Character::GetPosition() {
    return associated.box.Center();
}

bool Character::IsPlayingDead() {
    return isPlayingDead;
}

int Character::GetHP() {
    return hp;
}

void Character::Render() {}