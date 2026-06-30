#ifndef MELEEATTACK_H
#define MELEEATTACK_H

#include "component.h"
#include "timer.h"
#include <memory>
#include <vector>

class MeleeAttack : public Component {
public:
    // directionX: 1 (direita), -1 (esquerda). directionY: -1 (cima), 0 (frente)
    MeleeAttack(GameObject& associated, std::weak_ptr<GameObject> player, float directionX, float directionY);

    void Update(float dt) override;
    void Render() override;
    void NotifyCollision(GameObject& other) override;

private:
    std::weak_ptr<GameObject> player;
    Timer durationTimer;
    std::vector<GameObject*> hitEnemies; // Armazena inimigos já atingidos para não dar dano duplo
    
    float dirX;
    float dirY;
    int damage;
    float knockbackForce;
};

#endif