#ifndef PARALLAXBG_H
#define PARALLAXBG_H

#include "component.h"
#include "sprite.h"

// Componente de background com 4 camadas em parallax.
// Cada camada é uma imagem PNG 320x192 escalada pra preencher a tela (1200x900).
// O deslocamento horizontal de cada camada é proporcional ao movimento da câmera,
// criando sensação de profundidade (camadas mais distantes se movem menos).
class ParallaxBackground : public Component {
public:
    ParallaxBackground(GameObject& associated);

    void Update(float dt) override {}
    void Render() override;

private:
    static const int NUM_LAYERS = 4;

    Sprite layers[NUM_LAYERS]; // default-constructed, abertos em Open()
    float  parallaxFactors[NUM_LAYERS];
};

#endif
