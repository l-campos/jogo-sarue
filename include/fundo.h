#ifndef FUNDO_INFINITO_H
#define FUNDO_INFINITO_H

#include "component.h"
#include "sprite.h"

class FundoInfinito : public Component {
public:
    FundoInfinito(GameObject& associated, std::string file);
    ~FundoInfinito();

    void Update(float dt) override;
    void Render() override;

private:
    Sprite* sprite;
};

#endif