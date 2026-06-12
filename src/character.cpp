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

Character* Character::player = nullptr;

Character::Character(GameObject& associated, std::string sprite) 
    : Component(associated), hp(100), linearSpeed(300.0f), isGrounded(false) {
    
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

    // Controle de stun
    bool canControl = damageCooldown.Get() >= 0.3f;
    if (canControl) speed.x = 0;

    bool isMoving = false;
    Animator* animator = associated.GetComponent<Animator>();
    
    // 1. PROCESSA A FILA DE COMANDOS
    while (!taskQueue.empty()) {
        Command cmd = taskQueue.front();
        taskQueue.pop();
        SpriteRenderer* spriteRenderer = associated.GetComponent<SpriteRenderer>();
        
        // Se estiver atordoado, joga o comando no lixo e ignora!
        if (!canControl) continue; 
        
        if (cmd.type == Command::MOVE) {
            // Só mexemos no eixo X! O eixo Y pertence à gravidade e ao pulo agora.
            speed.x = cmd.pos.x * linearSpeed;
            
            // Aproveita para virar o sprite
            if (speed.x < 0) spriteRenderer->SetFlip(SDL_FLIP_HORIZONTAL);

            else if (speed.x > 0) spriteRenderer->SetFlip(SDL_FLIP_NONE);   

        } 
        else if (cmd.type == Command::JUMP) {
            if (isGrounded) {
                speed.y = -JUMP_FORCE; // Aplica o pulo
                isGrounded = false;
            }
        }
    }

    speed.y += GRAVITY * dt;
    if (speed.y > MAX_FALL_SPEED) speed.y = MAX_FALL_SPEED;

    if (speed.x != 0 || speed.y != 0) {
        associated.box.x += speed.x * dt;
        associated.box.y += speed.y * dt;
        isMoving = true; // Ajuda na animação depois
    }

    // 4. LIMITES E CHÃO (O mesmo que fizemos no Sarue)
    isGrounded = false;
    
    // Limites laterais da tela/mapa
    if (associated.box.x < 0) associated.box.x = 0;
    if (associated.box.x + associated.box.w > 10000) associated.box.x = 10000 - associated.box.w;

    float groundLevel = 530.0f; // Ajuste para o chão da sua composição
    
    if (associated.box.y + associated.box.h >= groundLevel) { 
        associated.box.y = groundLevel - associated.box.h;
        speed.y = 0;
        isGrounded = true; 
    }
    
    if (animator) {
        if (isMoving) {
            animator->SetAnimation("walking");
        } else {
            animator->SetAnimation("idle");
        }
    }
    SpriteRenderer* spriteRenderer = associated.GetComponent<SpriteRenderer>();
    if (spriteRenderer != nullptr) {
        if (speed.x > 0) {
            spriteRenderer->SetFlip(SDL_FLIP_NONE);
        } else if (speed.x < 0) {
            spriteRenderer->SetFlip(SDL_FLIP_HORIZONTAL);
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
    
    if (other.GetComponent<Enemy>() != nullptr) {
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
}

Vec2 Character::GetPosition() {
    return associated.box.Center();
}

void Character::Render() {}