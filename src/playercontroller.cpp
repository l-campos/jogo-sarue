#include "playercontroller.h"
#include "inputmanager.h"
#include "camera.h"

PlayerController::PlayerController(GameObject& associated) : Component(associated), character (nullptr) {}

void PlayerController::Start() {
    character = associated.GetComponent<Character>();
}

void PlayerController::Update(float dt) {
    InputManager& input = InputManager::GetInstance();
    
    Vec2 direction(0, 0);
    if (input.IsKeyDown(SDLK_a) || input.IsKeyDown(LEFT_ARROW_KEY)) direction.x = -1;
    if (input.IsKeyDown(SDLK_d) || input.IsKeyDown(RIGHT_ARROW_KEY)) direction.x = 1;

    // Eixo vertical, usado só quando o Character está no modo escalada (cipó)
    if (input.IsKeyDown(SDLK_w) || input.IsKeyDown(UP_ARROW_KEY)) direction.y = -1;
    if (input.IsKeyDown(SDLK_s) || input.IsKeyDown(DOWN_ARROW_KEY)) direction.y = 1;

    character->Issue(Character::Command{Character::Command::MOVE, direction.x, direction.y});
    
    // -------------------------------------------------------------------------
    // LÓGICA DE AÇÕES DO JOGADOR
    // -------------------------------------------------------------------------
    
    // 1. Verifica se está segurando para baixo
    bool downPressed = input.IsKeyDown(SDLK_s) || input.IsKeyDown(DOWN_ARROW_KEY);

    // 2. ATAQUE: Só permite atacar se NÃO estiver segurando para baixo (fingindo de morto)
    if (input.MousePress(LEFT_MOUSE_BUTTON) && !downPressed) {
        // Verifica se o jogador está segurando 'W' para atacar para cima
        float attackDirY = 0;
        if (input.IsKeyDown(SDLK_w)) {
            attackDirY = -1; // -1 indica direção para cima no nosso sistema
        }
        
        character->Issue(Character::Command{Character::Command::ATTACK, direction.x, attackDirY});
    }
    
    // 3. DESCER DA PLATAFORMA: Segurando pra baixo + apertou espaço
    if (downPressed && input.KeyPress(SPACE_KEY)) {
        character->Issue(Character::Command{Character::Command::DROP_DOWN, 0, 0});
    } 
    // 4. PULO NORMAL: Só apertou espaço (sem segurar para baixo)
    else if (input.KeyPress(SPACE_KEY)) {
        character->Issue(Character::Command{Character::Command::JUMP, 0, 0});
    }

    // 5. FINGIR DE MORTO: Segurando pra baixo
    if (downPressed) {
        character->Issue(Character::Command{Character::Command::PLAY_DEAD, 0, 0});
    }

    // 6. DASH
    if (input.KeyPress(LEFT_SHIFT)) {
        character->Issue(Character::Command{Character::Command::DASH, 0, 0});
    }
}

void PlayerController::Render() {}
