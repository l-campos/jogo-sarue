#include <cmath>
#include "aicontroller.h"
#include "character.h"
#include "game.h"
#include "gameobject.h"
#include "bullet.h"
#include "stagestate.h"
#include "spriterenderer.h"
#include "animator.h"
#include "collider.h"
#include "sound.h"

int AIController::npcCounter = 0;

AIController::AIController(GameObject& associated) : Component(associated), hp(10) {
    SpriteRenderer* npcSprite = new SpriteRenderer(associated, "img/NPC.png", 3, 4);
    associated.AddComponent(npcSprite);

    Animator* animator = new Animator(associated);
    animator -> AddAnimation("walking", Animation(0, 5, 0.1f));
    animator -> AddAnimation("idle", Animation(6, 9, 0.15f));
    animator -> AddAnimation("dead", Animation(10, 11, 0.2f));
    associated.AddComponent(animator);
    
    Collider* collider = new Collider(associated);
    associated.AddComponent(collider);

    state = RESTING;
    npcCounter++;
}

AIController::~AIController(){
    npcCounter--;
}

void AIController::Update(float dt){
    if (Character::player == nullptr) return;    
    
    if (hp <= 0 && state != DEAD) {
        state = DEAD;
        
        Animator* animator = associated.GetComponent<Animator>();
        if (animator) animator->SetAnimation("dead");
        
        
        deathTimer.Restart();

        if (deathTimer.Get() > 0.2f) {
            associated.RequestDelete();
        }
        return; 
    }

    Animator* animator = associated.GetComponent<Animator>();
    if (state == RESTING){
        if (animator) animator->SetAnimation("idle");

        restTimer.Update(dt);
        
        if (restTimer.Get() > 2.0f){
            destination = Character::player->GetPosition();
            state = MOVING;
        }
    }

    else if (state == DEAD){
        deathTimer.Update(dt);
        if (deathTimer.Get() > 0.2f){
            associated.RequestDelete();
        }
        return;
    }
    else if (state == MOVING){
        if (animator) animator->SetAnimation("walking");

        Vec2 currentPos = associated.box.Center();

        float dx = destination.x - currentPos.x;
        float dy = destination.y - currentPos.y;
        float distance = std::sqrt(dx*dx + dy*dy);

        if (distance < 5.0f){
            Vec2 playerPos = Character::player->GetPosition();
            float shootAngle = std::atan2(playerPos.y - currentPos.y, playerPos.x - currentPos.x);

            GameObject* bulletGO = new GameObject();
            Bullet* bullet = new Bullet(*bulletGO, shootAngle, 400.0f, 15, 800.0f, true);
            bulletGO->AddComponent(bullet);
            
            bulletGO->box.x = currentPos.x - (bulletGO->box.w / 2.0f);
            bulletGO->box.y = currentPos.y - (bulletGO->box.w / 2.0f);

            Sound shootSound("audio/Range.wav");
            shootSound.Play(1);

            Game::GetInstance().GetCurrentState().AddObject(bulletGO);

            restTimer.Restart();
            state = RESTING;
        }
        else{
            float moveAngle = std::atan2(dy, dx);
            Vec2 velocity = Vec2(200.0f * dt, 0).Rotate(moveAngle);

            associated.box.x += velocity.x;
            associated.box.y += velocity.y;

            SpriteRenderer* spriteRenderer = associated.GetComponent<SpriteRenderer>();

            if (spriteRenderer){
                if (velocity.x < 0){
                    spriteRenderer->SetFlip(SDL_FLIP_HORIZONTAL);
                }
                else{
                    spriteRenderer->SetFlip(SDL_FLIP_NONE);
                }
            }
        }

    }

    Collider* collider = associated.GetComponent<Collider>();
    if (collider != nullptr) {
        collider->Update(dt);
    }
}

void AIController::NotifyCollision(GameObject& other) {
    Bullet* bullet = other.GetComponent<Bullet>();
    
    if (bullet != nullptr && !bullet->targetsPlayer) {
        hp -= bullet->GetDamage();
    }
}

void AIController::Damage(int damage) {
    hp -= damage;
    Sound hitSound("audio/Hit0.wav");
        hitSound.Play(1);
}

void AIController::Render(){};