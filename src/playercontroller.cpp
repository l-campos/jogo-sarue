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

    // NOVO: eixo vertical, usado só quando o Character está no modo escalada
    // (isScaling) — fora do cipó, o Character ignora esse valor no MOVE.
    if (input.IsKeyDown(SDLK_w) || input.IsKeyDown(UP_ARROW_KEY)) direction.y = -1;
    if (input.IsKeyDown(SDLK_s) || input.IsKeyDown(DOWN_ARROW_KEY)) direction.y = 1;

    character->Issue(Character::Command{Character::Command::MOVE, direction.x, direction.y});

    
    /* Ataque a distancia com projeteis
    if (input.MousePress(LEFT_MOUSE_BUTTON)) {
        float mouseX = input.GetMouseX() + Camera::pos.x;
        float mouseY = input.GetMouseY() + Camera::pos.y;

        character->Issue(Character::Command{Character::Command::ATTACK, mouseX, mouseY});
    }*/

    if (input.MousePress(LEFT_MOUSE_BUTTON)) {
        // Verifica se o jogador está segurando 'W' para atacar para cima
        float attackDirY = 0;
        if (input.IsKeyDown(SDLK_w)) {
            attackDirY = -1; // -1 indica direção para cima no nosso sistema
        }

        // CORREÇÃO: Passamos a direção exata da tecla, quem decide o lado parado é o Character
        character->Issue(Character::Command{Character::Command::ATTACK, direction.x, attackDirY});
    }
    // CORREÇÃO: JUMP agora só no SPACE. Antes também disparava com
    // UP_ARROW_KEY, mas essa tecla virou o input de "subir" no cipó — se
    // deixasse os dois, apertar pra cima durante a escalada chutaria o
    // saruê pra fora da vinha sem querer.
    if (input.KeyPress(SPACE_KEY)) {
        character->Issue(Character::Command{Character::Command::JUMP, 0, 0});
    }

    if (input.IsKeyDown(SDLK_s) || input.IsKeyDown(DOWN_ARROW_KEY)) {
        character->Issue(Character::Command{Character::Command::PLAY_DEAD, 0, 0});
    }

    if (input.KeyPress(LEFT_SHIFT)) {
        character->Issue(Character::Command{Character::Command::DASH, 0, 0});
    }
}

void PlayerController::Render() {}
