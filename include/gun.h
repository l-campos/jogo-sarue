#ifndef GUN_H
#define GUN_H

#include <memory>
#include "component.h"
#include "sound.h"
#include "timer.h"
#include "vec2.h"


class Gun : public Component {
public:
    Gun(GameObject& associated, std::weak_ptr<GameObject> character);

    void Update(float dt) override;
    void Render() override;
    void Shoot(Vec2 target);

private:
    Sound shotSound;
    Sound reloadSound;

    int cooldownState;

    Timer cdTimer;
    std::weak_ptr<GameObject> character;

    float angle;
};

#endif