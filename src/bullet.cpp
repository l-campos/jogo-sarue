#include "bullet.h"
#include "spriterenderer.h"
#include "collider.h"
#include "character.h"
#include "zombie.h"
#include "aicontroller.h"

Bullet::Bullet(GameObject& associated, float angle, float speed, int damage, float maxDistance, bool targetsPlayer) 
    : Component(associated), distanceLeft(maxDistance), damage(damage) {
    
    SpriteRenderer* sprite = new SpriteRenderer(associated, "img/Bullet.png", 1, 1);
    sprite->SetScale(1.0f, 1.0f);
    associated.AddComponent(sprite);
    
    Collider* collider = new Collider(associated);
    associated.AddComponent(collider);
    
    this->targetsPlayer = targetsPlayer;
    this->speed = Vec2(speed, 0).Rotate(angle);
}

void Bullet::Update(float dt) {
    associated.box.x += speed.x * dt;
    associated.box.y += speed.y * dt;
    associated.angleDeg = speed.Inclination() * (180.0f / M_PI);

    float distanceTraveled = Vec2(speed.x * dt, speed.y * dt).Magnitude();
    distanceLeft -= distanceTraveled;    

    if (distanceLeft <= 0) {
        associated.RequestDelete();
    }
}

void Bullet::Render() {}

void Bullet::NotifyCollision(GameObject& other) {
    if (other.GetComponent<Zombie>() != nullptr && !targetsPlayer) {
        other.GetComponent<Zombie>()->Damage(50);
        associated.RequestDelete();
    } 
    
    else if (other.GetComponent<AIController>() != nullptr && !targetsPlayer){
        other.GetComponent<AIController>()->Damage(50);
        associated.RequestDelete();
    }
    else if (other.GetComponent<Character>() != nullptr && targetsPlayer) {
        associated.RequestDelete();
    }
}

int Bullet::GetDamage() {
    return damage;
}