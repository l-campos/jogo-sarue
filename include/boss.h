#ifndef BOSS_H
#define BOSS_H

#include "component.h"
#include "tilemap.h"
#include "timer.h"
#include "vec2.h"

class Boss : public Component {
public:
    Boss(GameObject& associated, float startX, float startY, TileMap* map);
    void Update(float dt) override;
    void Render() override;
    void NotifyCollision(GameObject& other) override;

private:
    enum BossState { WAITING, SCREAMING, JUMPING, LANDED };
    BossState state;
    Timer stateTimer;
    Vec2 speed;
    TileMap* map;
};

#endif