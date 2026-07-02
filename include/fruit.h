#ifndef FRUIT_H
#define FRUIT_H

#include "component.h"
#include "gameobject.h"

class Fruit : public Component {
public:
    Fruit(GameObject& associated, float x, float y);
    
    void Update(float dt) override;
    void Render() override;
    void NotifyCollision(GameObject& other) override;
};

#endif