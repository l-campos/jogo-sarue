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
#include "meleeattack.h"

const float GRAVITY = 1500.0f;
const float JUMP_FORCE = 700.0f;
const float MOVE_SPEED = 300.0f;
const float MAX_FALL_SPEED = 800.0f;
const float DASH_SPEED = 1000.0f; // Velocidade do avanço
const float DASH_TIME = 0.2f;     // Duração do avanço
const float DASH_COOLDOWN_TIME = 0.5f; // Tempo para poder usar de novo

Character* Character::player = nullptr;

Character::Character(GameObject& associated, std::string sprite, TileMap* map)
    : Component(associated), hp(100), linearSpeed(300.0f), isGrounded(false), isDashing(false), isFacingLeft(false), map(map) {
    
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
            float passDirX = cmd.pos.x;
            
            // CORREÇÃO: Se não está segurando "A" ou "D", ataca pra onde está virado
            if (passDirX == 0) {
                passDirX = isFacingLeft ? -1.0f : 1.0f;
            }

            GameObject* attackObj = new GameObject();
            
            MeleeAttack* melee = new MeleeAttack(*attackObj, Game::GetInstance().GetCurrentState().GetObjectPtr(&associated), passDirX, cmd.pos.y);
            
            attackObj->AddComponent(melee);
            Game::GetInstance().GetCurrentState().AddObject(attackObj);
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
    
    // CORREÇÃO 3: Limitando a velocidade máxima de queda para evitar "tunelamento"
    if (speed.y > MAX_FALL_SPEED) {
        speed.y = MAX_FALL_SPEED;
    }

    // CAPTURA A CAIXA REAL DE COLISÃO
    Collider* collider = associated.GetComponent<Collider>();
    Rect hitbox = (collider != nullptr) ? collider->box : associated.box;

    // ----- 1. MOVIMENTO E COLISÃO NO EIXO X -----
    associated.box.x += speed.x * dt;
    hitbox.x += speed.x * dt; // Simula onde a hitbox vai estar

    if (map != nullptr) {
        int left = std::floor(hitbox.x / 32.0f);
        int right = std::floor((hitbox.x + hitbox.w - 1.0f) / 32.0f);
        int top = std::floor(hitbox.y / 32.0f);
        int bottom = std::floor((hitbox.y + hitbox.h - 1.0f) / 32.0f);

        bool collisionX = false;
        if (speed.x > 0) { // Andando para a direita
            // CORREÇÃO 2: Checa todos os tiles da cabeça aos pés
            for (int y = top; y <= bottom; ++y) {
                if (map->IsSolid(right, y)) { collisionX = true; break; }
            }
            if (collisionX) {
                float diferencaHitbox = hitbox.x - associated.box.x;
                associated.box.x = (right * 32.0f) - hitbox.w - diferencaHitbox - 0.1f;
                hitbox.x = associated.box.x + diferencaHitbox; // CORREÇÃO 1: Sincroniza a hitbox
            }
        } else if (speed.x < 0) { // Andando para a esquerda
            for (int y = top; y <= bottom; ++y) {
                if (map->IsSolid(left, y)) { collisionX = true; break; }
            }
            if (collisionX) {
                float diferencaHitbox = hitbox.x - associated.box.x;
                associated.box.x = (left * 32.0f) + 32.0f - diferencaHitbox + 0.1f;
                hitbox.x = associated.box.x + diferencaHitbox; // CORREÇÃO 1: Sincroniza a hitbox
            }
        }
    }

    // ----- 2. MOVIMENTO E COLISÃO NO EIXO Y -----
    associated.box.y += speed.y * dt;
    hitbox.y += speed.y * dt;
    isGrounded = false;

    if (map != nullptr) {
        // Recalcula após o movimento no X (a hitbox agora está no lugar certo)
        int left = std::floor(hitbox.x / 32.0f);
        int right = std::floor((hitbox.x + hitbox.w - 1.0f) / 32.0f);
        int top = std::floor(hitbox.y / 32.0f);
        int bottom = std::floor((hitbox.y + hitbox.h - 1.0f) / 32.0f);

        bool collisionY = false;
        if (speed.y > 0) { // A cair
            // CORREÇÃO 2: Checa todos os tiles da esquerda à direita da hitbox
            for (int x = left; x <= right; ++x) {
                if (map->IsSolid(x, bottom)) { collisionY = true; break; }
            }
            if (collisionY) {
                float diferencaHitbox = hitbox.y - associated.box.y;
                associated.box.y = (bottom * 32.0f) - hitbox.h - diferencaHitbox - 0.1f;
                hitbox.y = associated.box.y + diferencaHitbox; // CORREÇÃO 1
                speed.y = 0;
                isGrounded = true;
            }
        } else if (speed.y < 0) { // A saltar e a bater com a cabeça
            for (int x = left; x <= right; ++x) {
                if (map->IsSolid(x, top)) { collisionY = true; break; }
            }
            if (collisionY) {
                float diferencaHitbox = hitbox.y - associated.box.y;
                associated.box.y = (top * 32.0f) + 32.0f - diferencaHitbox + 0.1f;
                hitbox.y = associated.box.y + diferencaHitbox; // CORREÇÃO 1
                speed.y = 0;
            }
        }
    }

    if (associated.box.y > 10000.0f) { // Caiu muito no abismo (ajuste o valor se seu mapa for muito fundo)
        hp = 0; // O Saruê morre
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