#ifndef FUNDO_INFINITO_H
#define FUNDO_INFINITO_H

#include "component.h"
#include "sprite.h"

class FundoInfinito : public Component {
public:
    // Adicionamos scaleX e scaleY para esticar as imagens de 320x192 para a tela toda
    FundoInfinito(GameObject& associated, std::string file, float parallaxFactor, float scaleX = 1.0f, float scaleY = 1.0f);
    ~FundoInfinito();

    void Update(float dt) override;
    void Render() override;

private:
    Sprite* sprite;
    float parallaxFactor; 
};
#endif