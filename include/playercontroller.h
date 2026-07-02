#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H 

#include "component.h"
#include "character.h"

class PlayerController : public Component {
public:
    PlayerController(GameObject& associated);

    void Start() override;
    void Update(float dt) override;
    void Render() override;

private:
    Character* character; 
};

#endif