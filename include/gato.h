#ifndef GATO_H
#define GATO_H

#include "component.h"
#include "vec2.h"
#include "timer.h"
#include "gameobject.h"
#include "tilemap.h"

class Gato : public Component {
public:
    Gato(GameObject& associated, float x, float y, TileMap* map);
    
    void Update(float dt) override;
    void Render() override;
    void NotifyCollision(GameObject& other) override;

    void Damage(int damage, Vec2 attackerPos);

private:
    // A Máquina de Estados do Gato
    enum GatoState { PATROL, CHASE, JUMP, ATTACK, COOLDOWN };
    GatoState state;

    int hp;
    float startX;
    Vec2 speed;

    Timer cooldownTimer;
    
    bool isGrounded;
    float groundLevel;

    Timer damageCooldown;
    bool isStunned;

    TileMap* map;
};

#endif