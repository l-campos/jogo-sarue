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

// ============================================================================
// CONFIGURAÇÃO DA SPRITESHEET DO GATO (gato_spritesheet.png, 455x140px)
// ============================================================================
// Analisei a imagem que vocês mandaram: ela tem 3 linhas (correndo / atacando
// normal / atacando pulando) mas NEM TODAS as linhas têm o mesmo número de
// poses — a linha de "correndo" parece ter menos frames que as outras duas.
// Como o motor (Sprite::SetFrame) usa uma largura de célula ÚNICA pra imagem
// inteira, o numero de colunas usado abaixo (GATO_COLS) precisa ser igual ao
// da linha com MAIS frames (achei 7), e as linhas mais curtas só usam um
// intervalo menor de colunas, sobrando células vazias no final — sem problema,
// elas nunca chegam a ser desenhadas porque o frameEnd da Animation pára antes.
//
// Com 455px de largura e 7 colunas, cada frame fica com ~65px de largura.
// CONFIRA no jogo se as poses estão sendo cortadas — se sim, é sinal de que o
// numero de colunas real é diferente do que detectei e GATO_COLS precisa mudar.
const int GATO_COLS = 7; // <-- baseado em análise automática da imagem, confirmar visualmente
const int GATO_ROWS = 3;


Gato::Gato(GameObject& associated, float x, float y)
    : Component(associated), state(PATROL), hp(2), startX(x), isGrounded(false), groundLevel(700.0f), isStunned(false) {
    associated.box.x = x;
    associated.box.y = y;
    speed = {PATROL_SPEED, 0};
    
    SpriteRenderer* sprite = new SpriteRenderer(associated, "img/gato.png", GATO_COLS, GATO_ROWS);
    sprite->SetFrameSize(37, 37);
    sprite->SetScale(3.0f, 3.0f); // Ajuste de escala para o tamanho desejado
    associated.AddComponent(sprite);

    Animator* animator = new Animator(associated);

    // Linha 1 (índice 0): correndo — usada em PATROL e CHASE.
    // AJUSTAR a quantidade real de frames úteis dessa linha (estimei 4 abaixo).
    animator->AddAnimation("running", Animation(0, 3, 0.1f));

    // Linha 2 (índice 1): atacando (normal/no chão) — usada em COOLDOWN, como
    // uma pose de "respirando"/recolhendo a garra depois do bote.
    // AJUSTAR quantidade real de frames (estimei 5 abaixo).
    animator->AddAnimation("attack_ground", Animation(7, 12, 0.12f));

    // Linha 3 (índice 2): atacando pulando — usada durante JUMP e ATTACK
    // (o gato no ar, indo pra cima e descendo com a garra esticada).
    animator->AddAnimation("attack_jump", Animation(15, 21, 0.1f));

    associated.AddComponent(animator);

    Collider* collider = new Collider(associated);
    associated.AddComponent(collider);
}

void Gato::Update(float dt) {
    if (isStunned) {
        damageCooldown.Update(dt);
        if (damageCooldown.Get() >= 0.3f) {
            isStunned = false; // Sai do atordoamento
        }
    }

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
    if (!isStunned) {
        if (state == PATROL) {
            if (animator) animator->SetAnimation("running");

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
            if (animator) animator->SetAnimation("running");

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
            if (animator) animator->SetAnimation("attack_jump");

            // Assim que ele atinge o pico do pulo e começa a cair, ele ativa o ataque
            if (speed.y > -100.0f) { 
                state = ATTACK;
                // ESTICA O COLISOR PARA SIMULAR A GARRA (Aumenta a largura em 50%)
                if (collider) collider->SetScale({1.5f, 1.0f}); 
            }
        }
        
        else if (state == ATTACK) {
            if (animator) animator->SetAnimation("attack_jump");
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
            if (animator) animator->SetAnimation("attack_ground");

            cooldownTimer.Update(dt);
            if (cooldownTimer.Get() > 1.0f) { // Fica 1 segundo parado no chão
                state = CHASE; // Volta a correr implacavelmente!
            }
        }
    }

    // 4. ATUALIZA COLISOR
    if (collider != nullptr) collider->Update(dt);

    // Se o objeto ficar 500 pixels para trás da câmera, deleta-o para salvar memória!
    if (associated.box.x < Camera::pos.x - 500.0f) {
        associated.RequestDelete();
    }
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

void Gato::Damage(int damage, Vec2 attackerPos) {
    // CORREÇÃO: Verifica apenas se o gato já não está atordoado
    if (!isStunned) { 
        hp -= damage;
        isStunned = true;
        damageCooldown.Restart();

        // O Knockback (joga pra cima e pra trás igual o player)
        speed.y = -300.0f;
        if (associated.box.Center().x > attackerPos.x) {
            speed.x = 400.0f;
        } else {
            speed.x = -400.0f;
        }
        isGrounded = false;

        if (hp <= 0) associated.RequestDelete();
    }
}
