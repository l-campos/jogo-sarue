#ifndef COLLIDER_H
#define COLLIDER_H

#include "component.h"
#include "vec2.h"
#include "rect.h"

class Collider : public Component {
public:
    Collider(GameObject& associated, Vec2 scale = Vec2(1, 1), Vec2 offset = Vec2(0, 0));
    
    Rect box;
    
    void Update(float dt) override;
    void Render() override;
    void SetScale(Vec2 scale);
    void SetOffset(Vec2 offset);

private:
    Vec2 scale;
    Vec2 offset;
};

#endif