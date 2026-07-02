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
            speed.y = -200.0f; 
        }
    }
    
    Animator* animator = associated.GetComponent<Animator>();
    SpriteRenderer* sprite = associated.GetComponent<SpriteRenderer>();

    // ------------------------------------------------------------------
    // LÓGICA DE ESTADOS DA INTELIGÊNCIA ARTIFICIAL (agora kamikaze):
    // PATROL -> vê o jogador, trava a posição dele -> DIVE (mergulho reto,
    // sem reajustar) -> chega no alvo/chão -> EXPLODE -> deleta.
    // ------------------------------------------------------------------
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
            
            float distX = std::abs(associated.box.Center().x - playerPos.x);
            
            if (distX < 100.0f && playerPos.y > associated.box.y) {
                // NOVO: trava o alvo AGORA. Depois disso o mergulho não
                // reage mais a onde o jogador for (se ele pular, foge, etc,
                // o pombo continua indo pro ponto que ele viu).
                diveTarget = playerPos;

                state = DIVE; 
                
                Vec2 direction = (diveTarget - associated.box.Center()).Normalize();
                speed = direction * DIVE_SPEED;

                if (sprite) {
                    if (speed.x < 0) sprite->SetFlip(SDL_FLIP_HORIZONTAL);
                    else sprite->SetFlip(SDL_FLIP_NONE);
                }
            }
        }
    } 
    
    else if (state == DIVE) {
        if (animator) animator->SetAnimation("walking"); 

        // Movimento em linha reta na direção travada — SEM reler a posição
        // atual do jogador, é só inércia da velocidade calculada no PATROL.
        associated.box.x += speed.x * dt;
        associated.box.y += speed.y * dt;

        Vec2 currentPos = associated.box.Center();
        float distToTarget = currentPos.Distance(diveTarget);

        // Chegou perto o suficiente do ponto travado, OU já passou dele e
        // bateu no chão (caso o jogador não estivesse mais lá) -> explode.
        float groundLevelPombo = 700.0f; // mesmo chão usado pelo Character
        bool reachedTarget = distToTarget < 24.0f;
        bool hitGround = (associated.box.y + associated.box.h) >= groundLevelPombo;

        if (reachedTarget || hitGround) {
            state = EXPLODE;
            explodeTimer.Restart();
            speed = Vec2(0, 0);
        }
    }
    
    else if (state == EXPLODE) {
        // Janela curtinha antes de sumir de fato — dá espaço pra, no futuro,
        // trocar por uma animação/partícula de explosão em vez de só deletar.
        // AJUSTAR: se a galera de design mandar uma animação de explosão,
        // troque a linha abaixo por animator->SetAnimation("explode") e ajuste
        // o tempo pra bater com a duração dos frames dela.
        explodeTimer.Update(dt);
        if (explodeTimer.Get() >= 0.15f) {
            associated.RequestDelete();
        }
        // Sai daqui sem atualizar o colisor/checar borda de tela — ele já não
        // se move mais nessa fase.
        return;
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
    // (O dano de contato já acontece do outro lado: Character::NotifyCollision
    // detecta o Enemy e aplica dano quando colide, então bater no jogador
    // durante o mergulho já funciona mesmo sem nada aqui.)
}

void Enemy::Damage(int damage, Vec2 attackerPos) {
    // CORREÇÃO: Verifica apenas se o pássaro já não está atordoado
    if (!isStunned && state != EXPLODE) { 
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
        
        // NOVO: sem RECOVER, então um hit durante o PATROL ou o DIVE só
        // aplica o knockback (via bloco isStunned no topo do Update) e depois
        // volta pro estado em que estava. Se preferirem que apanhar cancele o
        // mergulho, é só forçar "state = PATROL;" aqui.
        
        if (hp <= 0) associated.RequestDelete();
    }
}
