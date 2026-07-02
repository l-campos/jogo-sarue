#include "meleeattack.h"
#include "spriterenderer.h"
#include "animator.h"
#include "collider.h"
#include "enemy.h" 
#include "gato.h"
// REMOVIDO: #include "zombie.h" e #include "aicontroller.h" — sobra do outro
// jogo, não são usados em nenhum lugar deste arquivo.

// Precisa bater com os mesmos valores usados em character.cpp (SARUE_COLS /
// SARUE_ROWS / SARUE_FRAME_SIZE), já que o efeito usa a mesma spritesheet do
// saruê. Duplicado aqui em vez de compartilhado por header pra manter os dois
// arquivos independentes — se mudar um, lembre de mudar o outro.
const int EFFECT_SHEET_COLS = 4;
const int EFFECT_SHEET_ROWS = 10;
const int EFFECT_FRAME_SIZE = 32;
const int EFFECT_ROW = 5; // linha 6 (índice 5) = "efeito do ataque mordida"

MeleeAttack::MeleeAttack(GameObject& associated, std::weak_ptr<GameObject> player, float directionX, float directionY) 
    : Component(associated), player(player), dirX(directionX), dirY(directionY), damage(1), knockbackForce(150.0f) {
    // CORREÇÃO: "damage" estava em 50, mas em NotifyCollision() o dano aplicado
    // era sempre hardcoded como "1" (passaro->Damage(1, ...)), entao o valor
    // de 50 nunca era realmente usado — era código morto. Troquei o padrão pra
    // 1 e fiz NotifyCollision usar esse membro de verdade, pra dar pra ajustar
    // o dano da mordida num lugar só.

    // Efeito da mordida agora usa os frames 20-23 (linha 6) da própria
    // spritesheet do saruê, em vez do placeholder "img/Gun.png".
    // AJUSTAR o nome do arquivo se ele não se chamar "img/personagem.png" nos
    // assets finais do projeto (é o mesmo caminho usado em stagestate.cpp).
    SpriteRenderer* sprite = new SpriteRenderer(associated, "img/personagem.png", EFFECT_SHEET_COLS, EFFECT_SHEET_ROWS);
    sprite->SetFrameSize(EFFECT_FRAME_SIZE, EFFECT_FRAME_SIZE);
    sprite->SetScale(2.0f, 4.0f); // Mesma escala do saruê
    associated.AddComponent(sprite);

    // 4 frames (20 a 23), tocando rápido dentro da janela de 0.5s do ataque
    Animator* animator = new Animator(associated);
    int startFrame = EFFECT_ROW * EFFECT_SHEET_COLS;     // = 20
    int endFrame = startFrame + 3;                        // = 23
    animator->AddAnimation("attack", Animation(startFrame, endFrame, 0.0833f)); 
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

    if (dirY == -1) {
        // Ataque para cima (segurando W): o efeito nasce vertical na
        // spritesheet, entao aqui giramos 90° pra ficar deitado/horizontal,
        // como pedido.
        associated.box.x = playerPtr->box.Center().x - (associated.box.w / 2.0f);
        associated.box.y = playerPtr->box.y - associated.box.h;
        associated.angleDeg = 90.0f; // ERA -90; se ficar de cabeça pra baixo, troque pra -90 de novo
    } else {
        // Ataque para frente (padrão): mantém a orientação vertical natural do
        // efeito, parecido com o corte vertical do Hollow Knight.
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
        
        // CORREÇÃO: agora usa o membro "damage" de verdade, em vez do "1"
        // fixo que ignorava a configuração feita no construtor.
        if (passaro) passaro->Damage(damage, associated.box.Center());
        if (gato) gato->Damage(damage, associated.box.Center());
    }
}
