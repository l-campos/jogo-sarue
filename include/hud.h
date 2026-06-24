#ifndef HUD_H
#define HUD_H

#include "component.h"
#include "sprite.h"
#include "gameobject.h"

class HUD : public Component {
public:
    HUD(GameObject& associated);
    ~HUD();

    void Update(float dt) override;
    void Render() override;

private:
    Sprite* lifeIcon;
};

#endif