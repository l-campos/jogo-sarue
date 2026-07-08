#include "fundo.h"
#include "camera.h"
#include <cmath>

FundoInfinito::FundoInfinito(GameObject& associated, std::string file, float parallaxFactor, float scaleX, float scaleY) 
    : Component(associated), parallaxFactor(parallaxFactor) {
    sprite = new Sprite(file);
    sprite->SetScale(scaleX, scaleY); // Aplica a escala para cobrir a tela
}

FundoInfinito::~FundoInfinito() {
    delete sprite;
}

void FundoInfinito::Update(float dt) {}

void FundoInfinito::Render() {
    int width = sprite->GetWidth();
    if (width <= 0) return;

    // PARALLAX HORIZONTAL: Cada camada usa seu próprio fator individual
    float camX = Camera::pos.x * parallaxFactor;
    int startBloco = std::floor(camX / width);

    // PARALLAX VERTICAL: Todas as camadas usam o MESMO fator para não se separarem!
    // Fator 1.0f = Move exatamente junto com o chão
    // Fator 0.0f = Fica totalmente travado na tela
    // Fator 0.2f = Fundo sobe e desce um pouquinho quando você pula
    float fatorY = 0.2f; 
    float camY = Camera::pos.y * fatorY;

    // Loop para cobrir a tela (com folga para os lados)
    for (int i = 0; i < 5; i++) {
        float renderX = (startBloco + i) * width;
        float worldX = renderX + (Camera::pos.x - camX);
        
        float worldY = associated.box.y + (Camera::pos.y - camY); 

        sprite->Render(worldX, worldY);
    }
}