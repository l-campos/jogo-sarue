#include "meleeattack.h"
#include "spriterenderer.h"
#include "animator.h"
#include "collider.h"
#include "zombie.h"
#include "aicontroller.h"
#include "enemy.h" 
#include "gato.h"

MeleeAttack::MeleeAttack(GameObject& associated, std::weak_ptr<GameObject> player, float directionX, float directionY) 
    : Component(associated), player(player), dirX(directionX), dirY(directionY), damage(50), knockbackForce(150.0f) {
    
    // Configura o SpriteRenderer com a arma (3 colunas, 2 linhas = 6 frames)
    SpriteRenderer* sprite = new SpriteRenderer(associated, "img/Gun.png", 3, 2);
    associated.AddComponent(sprite);

    // Configura a animação para durar 0.5 segundos (0.5s / 6 frames = ~0.083f por frame)
    Animator* animator = new Animator(associated);
    animator->AddAnimation("attack", Animation(0, 5, 0.0833f)); 
    animator->SetAnimation("attack");
    associated.AddComponent(animator);

    // Adiciona o colisor para a hitbox
    Collider* collider = new Collider(associated);
    associated.AddComponent(collider);
}

void MeleeAttack::Update(float dt) {
    std::shared_ptr<GameObject> playerPtr = player.lock();

    // Se o jogador morrer ou sumir, o ataque some junto
    if (!playerPtr) {
        associated.RequestDelete();
        return;
    }

    // Acompanha o movimento do jogador
    float offsetDistance = 40.0f; // Distância do centro do saruê até o ataque

    if (dirY == -1) {
        // Ataque para cima
        associated.box.x = playerPtr->box.Center().x - (associated.box.w / 2.0f);
        associated.box.y = playerPtr->box.y - associated.box.h;
        associated.angleDeg = -90.0f; // Rotaciona a sprite para cima
    } else {
        // Ataque para os lados
        associated.angleDeg = 0.0f;
        associated.box.y = playerPtr->box.Center().y - (associated.box.h / 2.0f);

        SpriteRenderer* spriteRenderer = associated.GetComponent<SpriteRenderer>();
        if (dirX < 0) { // Olhando para a esquerda
            associated.box.x = playerPtr->box.x - associated.box.w;
            if (spriteRenderer) spriteRenderer->SetFlip(SDL_FLIP_HORIZONTAL);
        } else { // Olhando para a direita
            associated.box.x = playerPtr->box.x + playerPtr->box.w;
            if (spriteRenderer) spriteRenderer->SetFlip(SDL_FLIP_NONE);
        }
    }

    // Atualiza o timer de duração (0.5 segundos)
    durationTimer.Update(dt);
    if (durationTimer.Get() >= 0.5f) {
        associated.RequestDelete(); // Remove a hitbox da cena após meio segundo
    }
    
    Collider* collider = associated.GetComponent<Collider>();
    if (collider) collider->Update(dt);
}

void MeleeAttack::Render() {}

void MeleeAttack::NotifyCollision(GameObject& other) {
    // Verifica se já atingimos esse inimigo neste ataque
    for (GameObject* hitEnemy : hitEnemies) {
        if (hitEnemy == &other) return; 
    }

    Enemy* passaro = other.GetComponent<Enemy>();
    Gato* gato = other.GetComponent<Gato>();

    if (passaro != nullptr || gato != nullptr) {
        hitEnemies.push_back(&other); // Registra que apanhou
        
        // Aplica o dano chamando a função que vamos criar em cada um deles
        if (passaro) passaro->Damage(1, associated.box.Center());
        if (gato) gato->Damage(1, associated.box.Center());
    }
}