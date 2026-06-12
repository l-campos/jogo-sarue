#ifndef BULLET_H
#define BULLET_H

#include "component.h"
#include "vec2.h"

class Bullet : public Component {
public:
    Bullet(GameObject& associated, float angle, float speed, int damage, float maxDistance, bool targetsPlayer = false);
    
    void Update(float dt) override;
    void Render() override;
    void NotifyCollision(GameObject& other) override;
    
    int GetDamage();
    
    bool targetsPlayer;

private:
    Vec2 speed;
    
    float distanceLeft;
    
    int damage;
};

#endif