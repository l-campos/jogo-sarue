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
#include "tilemap.h"

const float GRAVITY = 1500.0f;     
const float JUMP_FORCE = 700.0f;   
const float MOVE_SPEED = 300.0f;   
const float MAX_FALL_SPEED = 800.0f;
const float DASH_SPEED = 1000.0f; // Velocidade do avanço
const float DASH_TIME = 0.2f;     // Duração do avanço
const float DASH_COOLDOWN_TIME = 0.5f; // Tempo para poder usar de novo

// NOVO: velocidade de subida/descida no cipó
const float CLIMB_SPEED = 150.0f;
// NOVO: força do "pulo de saída" ao apertar pulo durante a escalada
const float CLIMB_EXIT_JUMP = 500.0f;

// Duração da animação de mordida "presa" no Character (em segundos).
// Ajuste para casar com (numero_de_frames_da_linha5 * frameTime usado no AddAnimation abaixo).
const float ATTACK_ANIM_DURATION = 0.35f;

// Velocidade vertical do "salto de morte" (estilo Mario/Kirby caindo pra fora da tela)
const float DEATH_BUMP_SPEED = -350.0f;

// ============================================================================
// CONFIGURAÇÃO DA SPRITESHEET DO SARUÊ
// ============================================================================
// O Sprite::SetFrame() do motor usa UMA largura/altura de célula fixa pra imagem
// inteira (frameW = larguraTotal / SARUE_COLS), entao toda linha da spritesheet
// precisa ter o MESMO numero de colunas (mesmo que sobrem células vazias no fim
// de linhas mais curtas).
//
// CONFIRMADO com o spritesheet final (144x320px): 4 colunas, 10 linhas.
// Cada célula é 32x32px — mas a imagem tem 144px de largura (16px a mais por
// causa da cauda que extrapola na linha 9), então 144/4 = 36 ≠ 32. Por isso
// usamos jogador->SetFrameSize(32, 32) logo abaixo, pra forçar o tamanho real
// da célula em vez de deixar o Sprite calcular errado (era isso que causava
// o "escorregamento" da spritesheet durante a animação).
const int SARUE_COLS = 4;
const int SARUE_ROWS = 10;
const int SARUE_FRAME_SIZE = 32; // tamanho real de cada célula, em pixels

// Helper: calcula o índice de frame (linha-major) a partir de linha/coluna,
// do mesmo jeito que Sprite::SetFrame faz internamente (frameY = frame / cols).
inline int SF(int row, int col) { return row * SARUE_COLS + col; }

Character* Character::player = nullptr;

Character::Character(GameObject& associated, std::string sprite) 
    : Component(associated), hp(100), linearSpeed(300.0f), isGrounded(false), isPlayingDead(false), isDashing(false), isFacingLeft(false), 
    isAttacking(false), isBagging(false), isScaling(false), isHanging(false), isDying(false) {
    
    player = this;

    SpriteRenderer* jogador = new SpriteRenderer(associated, sprite, SARUE_COLS, SARUE_ROWS);
    // Força o recorte de cada frame pra 32x32 reais (ver comentário acima sobre
    // SARUE_FRAME_SIZE) — sem isso a spritesheet "escorrega" durante a animação.
    jogador->SetFrameSize(SARUE_FRAME_SIZE, SARUE_FRAME_SIZE);
    jogador->SetScale(4.0f, 4.0f);
    associated.AddComponent(jogador);

    Animator* animator = new Animator(associated);

    // Índices de frame conferidos via análise de pixel do PNG final (144x320,
    // 4 cols x 10 linhas). Algumas linhas têm menos de 4 poses desenhadas —
    // isso é normal, o grid é sempre 4 colunas mas cada Animation só usa o
    // intervalo de frames que a linha realmente tem.

    // Linha 1 (índice 0): Parado — só 1 pose desenhada (frame 0). Vira uma
    // "animação" de 1 frame só; frameTime não importa muito aqui.
    animator->AddAnimation("idle", Animation(SF(0, 0), SF(0, 0), 0.2f));

    // Linha 2 (índice 1): Andando — 4 poses (frames 4-7)
    animator->AddAnimation("walking", Animation(SF(1, 0), SF(1, 3), 0.1f));

    // Linha 3 (índice 2): Pulando — 4 poses (frames 8-11)
    animator->AddAnimation("jump", Animation(SF(2, 0), SF(2, 3), 0.12f));

    // Linha 4 (índice 3): Escalando cipó/vinha — 4 poses (frames 12-15)
    animator->AddAnimation("escala", Animation(SF(3, 0), SF(3, 3), 0.15f));

    // Linha 5 (índice 4): Ataque mordida — 4 poses (frames 16-19)
    animator->AddAnimation("attack", Animation(SF(4, 0), SF(4, 3), ATTACK_ANIM_DURATION / 4.0f));

    // Linha 6 (índice 5): Efeito do ataque, frames 20-23 — essa NÃO é tocada
    // aqui, ela pertence ao GameObject separado do MeleeAttack (ver meleeattack.cpp)

    // Linha 7 (índice 6): Recebendo dano — 4 poses (frames 24-27)
    animator->AddAnimation("hit", Animation(SF(6, 0), SF(6, 3), 0.1f));

    // Linha 8 (índice 7): Morrendo (caindo pra fora da tela) — 4 poses (frames 28-31)
    animator->AddAnimation("dead", Animation(SF(7, 0), SF(7, 3), 0.15f));

    // Linha 9 (índice 8): Fingindo de morto — 4 poses (frames 32-35). É a linha
    // com a cauda extrapolando; o último frame (35) fica com uns pixels da
    // cauda cortados pela SetFrameSize(32,32) — ver observação no topo do chat
    // sobre as 3 opções pra resolver isso definitivamente se incomodar.
    animator->AddAnimation("play", Animation(SF(8, 3), SF(8, 3), 0.25f));

    // Linha 10 (índice 9): Pendurado — só 1 pose desenhada (frame 36)
    animator->AddAnimation("pendura", Animation(SF(9, 0), SF(9, 0), 0.15f));

    // "bolsa" não tinha uma linha correspondente na descrição nova — deixei de
    // fora. Se a galera de design mandar essa pose, é só adicionar mais uma
    // linha e um AddAnimation igual aos outros.

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
    attackTimer.Update(dt);

    if (dashTimer.Get() > DASH_TIME) {
        isDashing = false;
    }

    // Controle de stun
    bool canControl = damageCooldown.Get() >= 0.3f;
    if (canControl) speed.x = 0;

    // Acabou a janela da animação de mordida? então solta a flag.
    if (isAttacking && attackTimer.Get() >= ATTACK_ANIM_DURATION) {
        isAttacking = false;
    }

    SpriteRenderer* spriteRenderer = associated.GetComponent<SpriteRenderer>();
    Animator* animator = associated.GetComponent<Animator>();
    
    isPlayingDead = false;

    // ------------------------------------------------------------------
    // ÁGUA e QUEDA DO MAPA
    // ------------------------------------------------------------------
    if (hp > 0) {
        Vec2 center = associated.box.Center();

        // CORREÇÃO: o limite de queda era 950px fixo, calibrado pra escala 2x.
        // Em escala 3.2+ o chão (tile row 9-10) fica além de 950px — o player
        // morria pelo fallback ANTES de pousar. O limite agora é dinâmico:
        // fundo do mapa + 200px de margem, que funciona em qualquer escala.
        float fallLimit = 950.0f; // fallback se não houver tilemap
        if (TileMap::instance != nullptr) {
            fallLimit = TileMap::instance->TileToPixelY(
                            TileMap::instance->GetHeight()) + 200.0f;
        }
        if (associated.box.y > fallLimit) hp = 0;

        // Tocou na camada de água (não mata se estiver escalando cipó)
        if (hp > 0 && TileMap::instance != nullptr && !isScaling) {
            bool onClimb = TileMap::instance->IsClimbAtPixel(center.x, center.y);
            if (!onClimb && TileMap::instance->IsWaterAtPixel(center.x, center.y)) {
                hp = 0;
            }
        }
    }

    // ------------------------------------------------------------------
    // MORTE (linha 8): aplica um pequeno "salto" de morte uma única vez e
    // deixa a gravidade levar o saruê pra fora da tela, igual Mario/Kirby.
    // Antes essa animação só tocava parada no lugar.
    // ------------------------------------------------------------------
    if (hp <= 0) {
        if (!isDying) {
            isDying = true;
            speed.x = 0;
            speed.y = DEATH_BUMP_SPEED; // pequeno impulso pra cima antes de cair
        }

        deathTimer.Update(dt);

        if (animator) {
            animator->SetAnimation("dead");
        }

        // Continua aplicando física de queda livre (sem checar chão) pra ele
        // sumir por baixo da tela.
        speed.y += GRAVITY * dt;
        associated.box.x += speed.x * dt;
        associated.box.y += speed.y * dt;

        // Deleta depois de cair um tempo OU sair bem abaixo da tela, o que vier primeiro.
        if (deathTimer.Get() >= 1.5f || associated.box.y > 1200.0f) {
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
            if (isScaling) {
                // NOVO: enquanto escalando, o eixo Y do comando MOVE controla
                // subir/descer no cipó (em vez de mexer em X, pra ele ficar
                // centralizado na vinha).
                speed.x = 0;
                speed.y = cmd.pos.y * CLIMB_SPEED;
            } else {
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
        } 
        else if (cmd.type == Command::JUMP) {
            if (isScaling) {
                // NOVO: pulo de saída do cipó — solta a escalada e dá um
                // impulso pra cima, pra "chegar do outro lado" como vocês pediram.
                isScaling = false;
                speed.y = -CLIMB_EXIT_JUMP;
                isGrounded = false;
            }
            else if (isGrounded) {
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

            // NOVO: avisa o próprio Character que está mordendo, pra Update()
            // poder escolher a animação "attack" (linha 5) nele mesmo.
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

    // ------------------------------------------------------------------
    // CIPÓ: entra no modo escalada se, no ar (depois de ter pulado), tocar
    // um tile da camada "Elementos de escalar". Sai sozinho se sair da
    // área do cipó (o botão de pulo também tira, ver Command::JUMP acima).
    // ------------------------------------------------------------------
    if (TileMap::instance != nullptr) {
        Vec2 center = associated.box.Center();
        bool onClimbTile = TileMap::instance->IsClimbAtPixel(center.x, center.y);

        if (!isScaling && !isGrounded && onClimbTile) {
            isScaling = true;
            speed = Vec2(0, 0);
        }
        else if (isScaling && !onClimbTile) {
            isScaling = false; // saiu do cipó por cima, por baixo ou pro lado
        }
    }

    if (isDashing) {
        if (isFacingLeft) speed.x = -DASH_SPEED; 
        else speed.x = DASH_SPEED;
        speed.y = 0; 
    }

    // Enquanto escalando não tem gravidade — ele flutua no cipó, controlado
    // só pelo input vertical processado no Command::MOVE acima.
    if (!isScaling) {
        speed.y += GRAVITY * dt;
        if (speed.y > MAX_FALL_SPEED) speed.y = MAX_FALL_SPEED;
    }

    if (speed.x != 0 || speed.y != 0) {
        associated.box.x += speed.x * dt;
        associated.box.y += speed.y * dt;
    }

    isGrounded = false;
    
    // Limites laterais da tela/mapa
    if (associated.box.x < 0) associated.box.x = 0;

    // ------------------------------------------------------------------
    // COLISÃO SÓLIDA COM O TILEMAP (substitui o "groundLevel = 700.0f" fixo
    // de antes). Checagem simples: se o pé do saruê está descendo (speed.y
    // >= 0) e o ponto embaixo dos pés cai num tile sólido, encaixa ele em
    // cima do tile e trava a queda.
    //
    // OBS: isso só resolve colisão VERTICAL (chão/teto). Colisão lateral
    // contra paredes de tile ainda não existe — hoje só existe colisão
    // lateral entre GameObjects com Collider (inimigos, etc). Se acontecer
    // de precisar de paredes sólidas de verdade, é um próximo passo à parte.
    // ------------------------------------------------------------------
    if (TileMap::instance != nullptr && !isScaling) {
        float feetY = associated.box.y + associated.box.h;
        float centerX = associated.box.Center().x;

        if (speed.y >= 0 && TileMap::instance->IsSolidAtPixel(centerX, feetY)) {
            int tileY = TileMap::instance->PixelToTileY(feetY);
            associated.box.y = TileMap::instance->TileToPixelY(tileY) - associated.box.h;
            speed.y = 0;
            isGrounded = true;
        }
    }
    
    bool isMoving = (speed.x != 0);

    // NOVO: "acabou de levar dano" — reaproveita o damageCooldown que já existia
    // pro stun, já que o limiar de 0.3s é exatamente a janela de invencibilidade
    // pós-hit. Isso liga a animação da linha 7 sem precisar de outro timer.
    bool justHit = damageCooldown.Get() < 0.3f;

    if (animator) {
        // Ordem de prioridade: dano > mordida > fingir de morto > pendurado >
        // escalando > pulando > andando > parado.
        if (justHit) {
            animator->SetAnimation("hit");
        }
        else if (isAttacking) {
            animator->SetAnimation("attack");
        }
        else if (isPlayingDead) {
            animator->SetAnimation("play");
        }
        else if (isHanging) {
            animator->SetAnimation("pendura");
        }
        else if (isScaling) {
            animator->SetAnimation("escala");
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
            damageCooldown.Restart(); // também reinicia a "janela" da animação "hit"

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