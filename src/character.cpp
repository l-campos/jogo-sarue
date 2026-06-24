#include "character.h"
#include "spriterenderer.h"
#include "gun.h"
#include "game.h"
#include "gameobject.h"
#include "animator.h"
#include "stagestate.h"
#include "collider.h"
#include "bullet.h"
#include "enemy.h"
#include "sound.h"
#include "fruit.h"
#include "gato.h"
#include "fumaca.h"

const float GRAVITY = 1500.0f;     
const float JUMP_FORCE = 700.0f;   
const float MOVE_SPEED = 300.0f;   
const float MAX_FALL_SPEED = 800.0f;
const float DASH_SPEED = 1000.0f; // Velocidade do avanço
const float DASH_TIME = 0.2f;     // Duração do avanço
const float DASH_COOLDOWN_TIME = 0.5f; // Tempo para poder usar de novo

Character* Character::player = nullptr;

Character::Character(GameObject& associated, std::string sprite) 
    : Component(associated), hp(100), linearSpeed(300.0f), isGrounded(false), isDashing(false), isFacingLeft(false) {
    
    player = this;

    SpriteRenderer* jogador = new SpriteRenderer(associated, sprite, 3, 4);
    associated.AddComponent(jogador);

    Animator* animator = new Animator(associated);
    animator -> AddAnimation("walking", Animation(0, 5, 0.1f));
    animator -> AddAnimation("idle", Animation(6, 9, 0.15f));
    animator -> AddAnimation("dead", Animation(10, 11, 0.2f));
    associated.AddComponent(animator);
    animator -> SetAnimation("idle");
    
    Collider* collider = new Collider(associated);
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

    if (dashTimer.Get() > DASH_TIME) {
        isDashing = false;
    }
    
    // Controle de stun
    bool canControl = damageCooldown.Get() >= 0.3f;
    if (canControl) speed.x = 0;

    SpriteRenderer* spriteRenderer = associated.GetComponent<SpriteRenderer>();
    Animator* animator = associated.GetComponent<Animator>();
    
    isPlayingDead = false;

    if (hp <= 0) {
        deathTimer.Update(dt);
        Animator* animator = associated.GetComponent<Animator>();
        
        if (animator) {
            animator->SetAnimation("dead");
        }

        if (deathTimer.Get() >= 1.0f) {
            associated.RequestDelete();
        }
        return;
    }
    
    // 1. PROCESSA A FILA DE COMANDOS
    while (!taskQueue.empty()) {
        Command cmd = taskQueue.front();
        taskQueue.pop();
        
        // Se estiver atordoado, joga o comando no lixo e ignora!
        if (!canControl) continue; 
        
        if (cmd.type == Command::PLAY_DEAD){
            if (isGrounded) {
                isPlayingDead = true;
                speed.x = 0;
            }
        }
        
        else if (cmd.type == Command::MOVE) {
            // Só mexemos no eixo X! O eixo Y pertence à gravidade e ao pulo agora.
            speed.x = cmd.pos.x * linearSpeed;
            
            // Aproveita para virar o sprite
            if (speed.x < 0) {
                spriteRenderer->SetFlip(SDL_FLIP_HORIZONTAL);
                isFacingLeft = true;
            }

            else if (speed.x > 0) {
                spriteRenderer->SetFlip(SDL_FLIP_NONE);   
                isFacingLeft = false;
            }

        } 
        else if (cmd.type == Command::JUMP) {
            if (isGrounded) {
                speed.y = -JUMP_FORCE; // Aplica o pulo
                isGrounded = false;
            }
        }
        else if (cmd.type == Command::ATTACK) {
            bool isEnemyBullet = false;
            GameObject* bulletObj = new GameObject();
            bulletObj->box.x = associated.box.Center().x;
            bulletObj->box.y = associated.box.Center().y;


            Bullet* bullet = new Bullet(*bulletObj, 0.0, 1000.0f, 10, 800.0f, isEnemyBullet);
            bulletObj->AddComponent(bullet);
            bulletObj->box.x -= bulletObj->box.w / 2.0f;
            bulletObj->box.y -= bulletObj->box.h / 2.0f;

            Game::GetInstance().GetCurrentState().AddObject(bulletObj);
        }
        else if (cmd.type == Command::DASH && !isPlayingDead) {
            if(!isDashing && dashCooldown.Get() > DASH_COOLDOWN_TIME){
                isDashing = true;
                dashTimer.Restart();
                dashCooldown.Restart();
                
                /*
                DESCOMENTAR PARA DEBUGAR, NAO FUNCIONANDO CÓDIGO DA FUMACA

                State& state = Game::GetInstance().GetCurrentState();
                GameObject* smokeObj = new GameObject();
                
                SpriteRenderer* smokeSprite = new SpriteRenderer(*smokeObj, "img/fumaca.png");
                
                if (isFacingLeft) smokeSprite->SetFlip(SDL_FLIP_HORIZONTAL);
                smokeObj->AddComponent(smokeSprite);

                smokeObj->box.x = associated.box.x;
                smokeObj->box.y = associated.box.y;

                Fumaca* fumaca = new Fumaca(*smokeObj);
                smokeObj->AddComponent(fumaca);
                
                state.AddObject(smokeObj);*/ 
            }
        }
    }

    if (isDashing) {
        if (isFacingLeft) speed.x = -DASH_SPEED; 
        else speed.x = DASH_SPEED;
        speed.y = 0; 
    }

    speed.y += GRAVITY * dt;
    if (speed.y > MAX_FALL_SPEED) speed.y = MAX_FALL_SPEED;

    if (speed.x != 0 || speed.y != 0) {
        associated.box.x += speed.x * dt;
        associated.box.y += speed.y * dt;
    }

    isGrounded = false;
    
    // Limites laterais da tela/mapa
    if (associated.box.x < 0) associated.box.x = 0;
    if (associated.box.x + associated.box.w > 10000) associated.box.x = 10000 - associated.box.w;

    float groundLevel = 700.0f; 
    
    if (associated.box.y + associated.box.h >= groundLevel) { 
        associated.box.y = groundLevel - associated.box.h;
        speed.y = 0;
        isGrounded = true; 
    }
    
    bool isMoving = (speed.x != 0);

    if (animator) {
        if (isPlayingDead) {
            animator->SetAnimation("dead");
        }
        else if (isMoving) {
            animator->SetAnimation("walking");
        } else {
            animator->SetAnimation("idle");
        }
    }

    Collider* collider = associated.GetComponent<Collider>();
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
        if (hp > 5) hp = 5;
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