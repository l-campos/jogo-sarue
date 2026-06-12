#ifndef ENEMY_H
#define ENEMY_H

#include "component.h"
#include "vec2.h"
#include "timer.h"

class Enemy : public Component {
public:
    Enemy(GameObject& associated, float startX, float startY);
    
    void Update(float dt) override;
    void Render() override;
    void NotifyCollision(GameObject& other) override;

private:
    enum EnemyState { PATROL, DIVE, RECOVER };
    EnemyState state;
    
    Vec2 speed;
    float startX; // Ponto inicial da patrulha
    float startY; // A altura que ele gosta de voar
};

#endif