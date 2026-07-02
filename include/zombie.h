#ifndef ZOMBIE_H
#define ZOMBIE_H

#include "component.h"
#include "sound.h"
#include "timer.h"

class Zombie : public Component {
public:
    Zombie(GameObject& associated);

    void Damage(int damage);
    void Update(float dt) override;
    void Render() override;
    void NotifyCollision(GameObject& other);
    
    static int zombieCount; 

private:
    int hitpoints; 
    bool hit;

    Sound deathSound;
    Sound hitSound;
    
    Timer hitTimer;
    Timer deathTimer;
};

#endif