#include "hud.h"
#include "character.h"
#include "camera.h"

HUD::HUD(GameObject& associated) : Component(associated){
    lifeIcon = new Sprite("img/vida.png");
    lifeIcon->SetScale(0.5f, 0.5f);
}

HUD::~HUD() {
    delete lifeIcon;
}

void HUD::Update(float dt){}

void HUD::Render() {
    if (Character::player != nullptr) {
        int hp = Character::player->GetHP();
        float startX = 20.0f;
        float startY = 20.0f;
        float spacing = 40.0f;

        for (int i = 0; i < hp; i++) {
            float x = startX + (i*spacing) + Camera::pos.x;
            float y = startY + Camera::pos.y;
            lifeIcon -> Render(x, y);
        }
    }
}