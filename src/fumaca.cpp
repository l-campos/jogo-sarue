#include "fumaca.h"
#include "spriterenderer.h"

Fumaca::Fumaca(GameObject& associated) : Component(associated) {
}

void Fumaca::Update(float dt) {
    lifeTimer.Update(dt);
    
    // A fumaça dura apenas 0.3 segundos na tela e depois deleta-se sozinha!
    if (lifeTimer.Get() > 2.0f) {
        associated.RequestDelete();
    }
}

void Fumaca::Render() {}