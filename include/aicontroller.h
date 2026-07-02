#ifndef AICONTROLLER_H
#define AICONTROLLER_H

#include "timer.h"
#include "vec2.h"
#include "component.h"
#include "sound.h"

class AIController : public Component{
public:
    AIController(GameObject& associated);
    ~AIController();
    
    void Update(float dt) override;
    void Render() override;
    void NotifyCollision(GameObject& other) override;
    void Damage(int damage);
    
    static int npcCounter;

private:
    enum AIState { MOVING, RESTING, DEAD };
    int hp;
    AIState state;
    Timer restTimer;
    Timer deathTimer;
    Vec2 destination;
};

#endif