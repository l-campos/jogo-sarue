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
        // Posição central do personagem
        float playerX = focus->box.Center().x;
        float playerY = focus->box.Center().y;
        
        // Posição central da tela
        float screenCenterX = pos.x + 600.0f; // Metade da tela de 1200
        float screenCenterY = pos.y + 450.0f; // Metade da tela de 900
        
        // Zona Morta (Deadzone) de 10% a partir do centro
        float deadzoneX = 1200.0f * 0.05f; // 120 pixels livres para os lados
        float deadzoneY = 900.0f * 0.10f;  // 90 pixels livres para cima/baixo
        
        // Empurra a câmera se o personagem sair dos 10% centrais (Horizontal)
        if (playerX > screenCenterX + deadzoneX) {
            pos.x += playerX - (screenCenterX + deadzoneX);
        } else if (playerX < screenCenterX - deadzoneX) {
            pos.x += playerX - (screenCenterX - deadzoneX);
        }
        
        // Empurra a câmera se o personagem sair dos 10% centrais (Vertical)
        if (playerY > screenCenterY + deadzoneY) {
            pos.y += playerY - (screenCenterY + deadzoneY);
        } else if (playerY < screenCenterY - deadzoneY) {
            pos.y += playerY - (screenCenterY - deadzoneY);
        }

    } else if (focus == nullptr) {
        InputManager& input = InputManager::GetInstance();
        // Trocado de KeyPress para IsKeyDown para movimentação fluida da câmera solta
        if (input.IsKeyDown(UP_ARROW_KEY)) {
            pos.y -= speed.y * dt;
        }
        if (input.IsKeyDown(DOWN_ARROW_KEY)) {
            pos.y += speed.y * dt;
        }
        if (input.IsKeyDown(LEFT_ARROW_KEY)) {
            pos.x -= speed.x * dt;
        }
        if (input.IsKeyDown(RIGHT_ARROW_KEY)) {
            pos.x += speed.x * dt;
        }
    }
}