#include "meleeattack.h"
#include "collider.h"
#include "enemy.h"
#include "gato.h"

// A duração do ataque deve casar com o que estava no Character (0.35s)
const float ATTACK_DURATION = 0.35f;
const float FRAME_TIME = ATTACK_DURATION / 4.0f; 

// =========================================================================
// CONFIGURAÇÕES VISUAIS DA CABEÇA DE ATAQUE (Fácil de alterar)
// =========================================================================
// 1. Aumente ou diminua a escala abaixo (ex: 3.5f, 5.0f) para mudar o tamanho:
const float HEAD_SCALE = 6.0f; 

// 2. Ajuste o quanto a cabeça vai para a FRENTE do rosto do Saruê:
const float HEAD_OFFSET_X = 1.0f; 

// 3. Ajuste a altura (0.0f fica no topo da caixa. Valores maiores a puxam para BAIXO):
const float HEAD_OFFSET_Y = -50.0f;  
// =========================================================================

MeleeAttack::MeleeAttack(GameObject& associated, std::weak_ptr<GameObject> player, float directionX, float directionY)
    : Component(associated), player(player), dirX(directionX), dirY(directionY), damage(1), knockbackForce(150.0f), currentFrame(0) {

    // 1. Instancia o Sprite da Cabeça com a nova escala configurável
    headSprite = new Sprite("img/saruebite.png", 4, 1);
    headSprite->SetScale(HEAD_SCALE, HEAD_SCALE);

    // 2. Instancia o Sprite do Efeito (hitbox)
    effectSprite = new Sprite("img/efeitobite.png", 4, 1);
    effectSprite->SetScale(4.0f, 4.0f);

    associated.box.w = effectSprite->GetWidth();
    associated.box.h = effectSprite->GetHeight();

    Collider* collider = new Collider(associated);
    associated.AddComponent(collider);
}

MeleeAttack::~MeleeAttack() {
    delete headSprite;
    delete effectSprite;
}

void MeleeAttack::Update(float dt) {
    std::shared_ptr<GameObject> playerPtr = player.lock();
    
    if (!playerPtr) {
        associated.RequestDelete();
        return;
    }

    frameTimer.Update(dt);
    if (frameTimer.Get() > FRAME_TIME) {
        currentFrame++;
        if (currentFrame > 3) currentFrame = 3; 
        headSprite->SetFrame(currentFrame);
        effectSprite->SetFrame(currentFrame);
        frameTimer.Restart();
    }

    if (dirY == -1) {
        associated.box.x = playerPtr->box.Center().x - (associated.box.w / 2.0f);
        associated.box.y = playerPtr->box.y - associated.box.h;
        associated.angleDeg = -90.0f; 
        effectSprite->SetFlip(SDL_FLIP_NONE);
    } else {
        associated.angleDeg = 0.0f;
        associated.box.y = playerPtr->box.Center().y - (associated.box.h / 2.0f);
        if (dirX < 0) { 
            associated.box.x = playerPtr->box.x - associated.box.w;
            effectSprite->SetFlip(SDL_FLIP_HORIZONTAL);
        } else { 
            associated.box.x = playerPtr->box.x + playerPtr->box.w;
            effectSprite->SetFlip(SDL_FLIP_NONE);
        }
    }

    durationTimer.Update(dt);
    if (durationTimer.Get() >= ATTACK_DURATION) {
        associated.RequestDelete(); 
    }
    
    Collider* collider = associated.GetComponent<Collider>();
    if (collider) collider->Update(dt);
}

void MeleeAttack::Render() {
    std::shared_ptr<GameObject> playerPtr = player.lock();
    
    if (playerPtr) {
        // Começa calculando o centro geométrico horizontal
        float headX = playerPtr->box.Center().x - (headSprite->GetWidth() / 2.0f);
        
        // Aplica o deslocamento de altura
        float headY = playerPtr->box.y + HEAD_OFFSET_Y; 

        if (dirY == -1) {
            // Se o ataque for para CIMA, a cabeça fica centralizada e sobe um pouco mais
            headSprite->SetFlip(dirX < 0 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
        } else {
            // Se o ataque for para a FRENTE, empurra a cabeça na direção do olhar usando o OFFSET_X
            if (dirX < 0) {
                headX -= HEAD_OFFSET_X; // Empurra para a esquerda
                headSprite->SetFlip(SDL_FLIP_HORIZONTAL);
            } else {
                headX += HEAD_OFFSET_X; // Empurra para a direita
                headSprite->SetFlip(SDL_FLIP_NONE);
            }
        }

        // Renderiza nas coordenadas ajustadas
        headSprite->Render(headX, headY);
    }

    effectSprite->Render(associated.box.x, associated.box.y, associated.angleDeg);
}

void MeleeAttack::NotifyCollision(GameObject& other) {
    for (GameObject* hitEnemy : hitEnemies) {
        if (hitEnemy == &other) return; 
    }

    Enemy* passaro = other.GetComponent<Enemy>();
    Gato* gato = other.GetComponent<Gato>();

    if (passaro != nullptr || gato != nullptr) {
        hitEnemies.push_back(&other); 
        
        if (passaro) passaro->Damage(damage, associated.box.Center());
        if (gato) gato->Damage(damage, associated.box.Center());
    }
}