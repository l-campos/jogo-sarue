#ifndef SPAWNER_H
#define SPAWNER_H

#include "component.h"
#include "timer.h"

class Spawner : public Component {
public:
    Spawner(GameObject& associated);
    
    void Update(float dt) override;
    void Render() override;

private:
    Timer pomboTimer;
    Timer gatoTimer;
    Timer frutaTimer;

    // Tempos que mudam para o jogo não ficar previsível
    float pomboCooldown;
    float gatoCooldown;
    float frutaCooldown;
};

#endif