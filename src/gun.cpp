#define _USE_MATH_DEFINES

#include <cmath>
#include "gun.h"
#include "spriterenderer.h"
#include "animator.h"
#include "stagestate.h"
#include "bullet.h"
#include "game.h"
#include "inputmanager.h"
#include "character.h"

Gun::Gun(GameObject& associated, std::weak_ptr<GameObject> character) 
    : Component(associated), shotSound("audio/Range.wav"), 
    reloadSound("audio/PumpAction.mp3"), cooldownState(0), character(character), angle(0) {

    SpriteRenderer* sprite = new SpriteRenderer(associated, "img/Gun.png", 3, 2);
    associated.AddComponent(sprite);

    Animator* animator = new Animator(associated);
    animator->AddAnimation("idle", Animation(0, 0, 1.0f));
    animator->AddAnimation("reloading", Animation(1, 3, 0.1f));
    animator->SetAnimation("idle");
    associated.AddComponent(animator);
}

void Gun::Update(float dt) {
    std::shared_ptr<GameObject> characterPtr = character.lock(); 

    if (!characterPtr) {
        associated.RequestDelete();
        return;
    }
    
    Vec2 charCenter = characterPtr->box.Center();
    float offset = 0.0f;

    InputManager& input = InputManager::GetInstance();
    Vec2 mousePos(input.GetMouseX() + Camera::pos.x, input.GetMouseY() + Camera::pos.y);

    float angleRad = associated.box.Center().Inclination(mousePos);
    associated.angleDeg = angleRad * (180.0f / M_PI);

    SpriteRenderer* spriteRenderer = associated.GetComponent<SpriteRenderer>();
    if (spriteRenderer != nullptr) {
        if (std::abs(angleRad) > M_PI / 2) {
            spriteRenderer->SetFlip(SDL_FLIP_VERTICAL);
            offset = -10.0f;
        } else {
            spriteRenderer->SetFlip(SDL_FLIP_NONE);
            offset = 10.0f;
        }
    }

    associated.box.x = charCenter.x - (associated.box.w / 2.0f) + offset; // Ajuste horizontal para posicionar a arma um pouco à direita do centro do personagem
    associated.box.y = charCenter.y - (associated.box.h / 2.0f) + 13.0f; // Ajuste vertical para posicionar a arma um pouco abaixo do centro do personagem

    if (cooldownState != 0) {
        cdTimer.Update(dt);
        Animator* animator = associated.GetComponent<Animator>();
        if (cooldownState == 1 && cdTimer.Get() >= 0.2f) {
            cooldownState = 2;
            cdTimer.Restart();
            reloadSound.Play(1);
            if (animator) animator -> SetAnimation("reloading");
        }

        if (cooldownState == 2 && cdTimer.Get() >= 0.3f) {
            cooldownState = 3;
            cdTimer.Restart();
            if (animator) animator -> SetAnimation("idle");
        }

        if (cooldownState == 3 && cdTimer.Get() >= 0.3f) {
            cooldownState = 0;
            cdTimer.Restart();
        }
    }
}

void Gun::Render() {}

void Gun::Shoot(Vec2 target) {
    if (cooldownState == 0) {
        shotSound.Play(1);
        cooldownState = 1;
        cdTimer.Restart();

        Vec2 gunCenter = associated.box.Center();
        float angle = gunCenter.Inclination(target);
        float offset = 15.0f * (M_PI / 180.0f); 
        float angles[3] = {angle, angle + offset, angle - offset};
        bool isEnemyBullet = false;
        
        for (int i = 0; i < 3; i++){
            GameObject* bulletObj = new GameObject();
            bulletObj->box.x = associated.box.Center().x;
            bulletObj->box.y = associated.box.Center().y;


            Bullet* bullet = new Bullet(*bulletObj, angles[i], 1000.0f, 10, 800.0f, isEnemyBullet);
            bulletObj->AddComponent(bullet);
            bulletObj->box.x -= bulletObj->box.w / 2.0f;
            bulletObj->box.y -= bulletObj->box.h / 2.0f;

            Game::GetInstance().GetCurrentState().AddObject(bulletObj);
        }
        
        if (Character::player != nullptr) {
            
            Character* gunOwner = character.lock()->GetComponent<Character>();
            
            if (gunOwner == Character::player) {
                isEnemyBullet = false; 
            }
        }
    }
}