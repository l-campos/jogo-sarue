#include "parallaxbg.h"
#include "camera.h"
#include <cmath>

ParallaxBackground::ParallaxBackground(GameObject& associated)
    : Component(associated) {

    // Ordem de desenho: índice 0 = mais longe (fundo), índice 3 = mais perto (frente).
    //
    // AJUSTAR OS NOMES: renomear os arquivos que o designer mandou para estes caminhos,
    // ou trocar os caminhos abaixo pelos nomes que vocês já usam na pasta img/:
    //
    //   BG_4_-_Cena_1_-_320x192.png         -> img/bg4.png  (ondas/céu, 0% transp.)
    //   BG_3_Predios_-_Fase_2_-_320x192.png -> img/bg3.png  (prédios, 45% transp.)
    //   BG_1_arvores_-_Fase_2_-_320x192.png -> img/bg1.png  (árvores esparsas, 59% transp.)
    //   BG_2_arvore_-_Fase_2_-_320x192.png  -> img/bg2.png  (vegetação densa, 94% transp.)
    const char* files[NUM_LAYERS] = {
        "img/bg4.png",  // céu / ondas — opaco, cobre tudo atrás
        "img/bg3.png",  // prédios distantes
        "img/bg1.png",  // árvores esparsas
        "img/bg2.png"   // vegetação densa (primeiro plano do BG)
    };

    // Fator de parallax: quanto a camada se move em relação à câmera.
    //   0.0 = completamente estática na tela
    //   1.0 = move igual à câmera (sem parallax = elemento de foreground)
    // Camadas mais distantes recebem valores menores.
    parallaxFactors[0] = 0.05f;  // céu: quase parado
    parallaxFactors[1] = 0.15f;  // prédios distantes: bem devagar
    parallaxFactors[2] = 0.30f;  // árvores: velocidade média
    parallaxFactors[3] = 0.50f;  // vegetação: mais próximo, mais rápido

    for (int i = 0; i < NUM_LAYERS; i++) {
        layers[i].Open(files[i]);
        // cameraFollower = true: vamos controlar a posição na tela manualmente,
        // sem deixar o Sprite subtrair Camera::pos de novo.
        layers[i].SetCameraFollower(true);
    }
}

void ParallaxBackground::Render() {
    const int SCREEN_W = 1200;
    const int SCREEN_H = 900;

    for (int i = 0; i < NUM_LAYERS; i++) {
        if (!layers[i].IsOpen()) continue;

        // Quanto a câmera se moveu × fator da camada = quanto esta camada rolou
        float scroll = Camera::pos.x * parallaxFactors[i];

        // Envolve dentro da largura de uma cópia pra criar o loop infinito
        float bgX = -fmodf(scroll, (float)SCREEN_W);
        if (bgX > 0) bgX -= SCREEN_W; // garante bgX no intervalo (-SCREEN_W, 0]

        // Duas cópias lado a lado cobrem os 1200px da tela independente do offset.
        // Cada cópia estica a imagem de 320x192 pra preencher 1200x900 (leve distorção
        // de aspecto aceitável em background).
        layers[i].Render((int)bgX,             0, SCREEN_W, SCREEN_H);
        layers[i].Render((int)bgX + SCREEN_W,  0, SCREEN_W, SCREEN_H);
    }
}
