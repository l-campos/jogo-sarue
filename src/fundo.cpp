#include "fundo.h"
#include "camera.h"
#include <cmath>

FundoInfinito::FundoInfinito(GameObject& associated, std::string file) : Component(associated) {
    sprite = new Sprite(file);
}

FundoInfinito::~FundoInfinito() {
    delete sprite;
}

void FundoInfinito::Update(float dt) {
    // Não precisa de atualizar físicas
}

void FundoInfinito::Render() {
    int width = sprite->GetWidth();
    if (width <= 0) return;

    // FATOR DE PARALLAX (Dica para o futuro):
    // 1.0f = Move-se como o chão normal (Ideal para o seu placeholder atual)
    // 0.5f = Move-se lentamente (Ideal para montanhas no fundo)
    // 0.0f = Estático no ecrã (Ideal para nuvens distantes ou céu)
    float parallaxFactor = 1.0f;

    // Descobre onde a câmara está baseada no parallax
    float camX = Camera::pos.x * parallaxFactor;

    // Descobre em qual "bloco" infinito a câmara está agora
    int startBloco = std::floor(camX / width);

    // Desenha o bloco atual e os dois próximos para garantir que preenchemos a tela toda
    for (int i = 0; i < 3; i++) {
        // A posição do bloco no "Mundo Falso"
        float renderX = (startBloco + i) * width;
        
        // A compensação matemática para o SDL desenhar no lugar exato do mundo real
        float worldX = renderX + (Camera::pos.x - camX);
        float worldY = associated.box.y;
        
        sprite->Render(worldX, worldY);
    }
}