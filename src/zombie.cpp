#include "zombie.h"
#include "gameobject.h"
#include "animator.h"
#include "spriterenderer.h"
#include "inputmanager.h"
#include "timer.h"
#include "camera.h"
#include "collider.h"
#include "bullet.h"
#include "character.h"

int Zombie::zombieCount = 0; 

Zombie::Zombie(GameObject & associated): Component(associated), hitpoints(200),
    hit(false),
    deathSound("audio/Dead.wav"),
    hitSound("audio/Hit0.wav") {

        SpriteRenderer* sprite = new SpriteRenderer(associated, "img/Enemy.png", 3, 2);
        associated.AddComponent(sprite);

        Collider* collider = new Collider(associated);
        associated.AddComponent(collider);

        Animator* animator = new Animator(associated);
        animator -> AddAnimation("walk", Animation(0, 3, 0.15f));
        animator -> AddAnimation("hit", Animation(4, 1, 0.1f));
        animator -> AddAnimation("die", Animation(5, 5, 0.2f));

        animator -> SetAnimation("walk");
        associated.AddComponent(animator);
        zombieCount++;
    }

void Zombie::Damage(int damage) {
    if (hitpoints <= 0) return;

    hitpoints -= damage;
    hitSound.Play(1);

    if (hitpoints <= 0) {
        Collider* collider = associated.GetComponent<Collider>();
        if (collider != nullptr) {
            associated.RemoveComponent(collider);
        }

        Animator* animator = associated.GetComponent <Animator> ();
        if (animator) animator -> SetAnimation("die");

        deathSound.Play(1);
        deathTimer.Restart();   

    } else {
        hit = true;
        hitTimer.Restart();
        Animator* animator = associated.GetComponent < Animator > ();
        if (animator) animator -> SetAnimation("hit");
    }
}

void Zombie::Update(float dt) {
    hitTimer.Update(dt);
    deathTimer.Update(dt);

    if (Character::player != nullptr) {
        Vec2 playerPos = Character::player->GetPosition();
        Vec2 zombiePos = associated.box.Center();

        float dx = playerPos.x - zombiePos.x;
        float dy = playerPos.y - zombiePos.y;
        float distance = sqrt(dx * dx + dy * dy); 

        if (distance > 5.0f) {
            float angle = std::atan2(dy, dx); 
            Vec2 speed = Vec2(150.0f * dt, 0).Rotate(angle);
            associated.box.x += speed.x;
            associated.box.y += speed.y;

            SpriteRenderer* sprite = associated.GetComponent<SpriteRenderer>();
            if (sprite != nullptr) {
                if (speed.x > 0) {
                    sprite->SetFlip(SDL_FLIP_NONE);
                } else if (speed.x < 0) {
                    sprite->SetFlip(SDL_FLIP_HORIZONTAL);
                }
            }
        }
    }

    if (hitpoints <= 0 && deathTimer.Get() > 0.2f) {
        associated.RequestDelete();
        zombieCount--;
        
        return;
    }

    if (hit && hitTimer.Get() > 0.5f) {
        hit = false;
        Animator* animator = associated.GetComponent < Animator > ();
        if (animator) animator -> SetAnimation("walk");
    }

    Collider* collider = associated.GetComponent<Collider>();
    if (collider != nullptr) {
        collider->Update(dt);
    }
    
}

void Zombie::NotifyCollision(GameObject& other) {
    Bullet* bullet = other.GetComponent<Bullet>();
    
    if (bullet != nullptr && !bullet->targetsPlayer) {
        hitpoints -= bullet->GetDamage();        
    }
}

void Zombie::Render() {}