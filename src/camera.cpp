#include "camera.h"
#include "inputmanager.h"

GameObject* Camera::focus = nullptr;
Vec2 Camera::pos(0, 0);
Vec2 Camera::speed(300, 300);

void Camera::Follow(GameObject* newFocus) {
    focus = newFocus;
}

void Camera::Unfollow() {
    focus = nullptr;
}

void Camera::Update(float dt) {
    if (focus != nullptr) {
        pos.x = focus->box.Center().x - 600.0f; // Metade da tela (1200)
        pos.y = focus->box.Center().y - 450.0f; // Metade da tela (900)
    } else if (focus == nullptr) {
        InputManager& input = InputManager::GetInstance();

        if (input.KeyPress(UP_ARROW_KEY)) {
            pos.y -= speed.y * dt;
        }
        if (input.KeyPress(DOWN_ARROW_KEY)) {
            pos.y += speed.y * dt;
        }
        if (input.KeyPress(LEFT_ARROW_KEY)) {
            pos.x -= speed.x * dt;
        }
        if (input.KeyPress(RIGHT_ARROW_KEY)) {
            pos.x += speed.x * dt;
        }
    }
}