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

    character->Issue(Character::Command{Character::Command::MOVE, direction.x, 0});

    // Descomentar pra implementar ataque
    /*if (input.MousePress(LEFT_MOUSE_BUTTON)) {
        float mouseX = input.GetMouseX() + Camera::pos.x;
        float mouseY = input.GetMouseY() + Camera::pos.y;

        character->Issue(Character::Command{Character::Command::SHOOT, mouseX, mouseY});
    }*/

    if (input.KeyPress(SPACE_KEY) || input.KeyPress(UP_ARROW_KEY)) {
        character->Issue(Character::Command{Character::Command::JUMP, 0, 0});
    }
}

void PlayerController::Render() {}