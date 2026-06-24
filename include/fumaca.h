#ifndef FUMACA_H
#define FUMACA_H

#include "component.h"
#include "timer.h"

class Fumaca : public Component {
public:
    Fumaca(GameObject& associated);
    void Update(float dt) override;
    void Render() override;

private:
    Timer lifeTimer;
};

#endif