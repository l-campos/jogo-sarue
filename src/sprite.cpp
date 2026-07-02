#define INCLUDE_SDL_IMAGE

#include <iostream>
#include "sprite.h"
#include "resources.h"
#include "game.h"
#include "camera.h"
#include "SDL_include.h"
#include "vec2.h"

using namespace std;

Sprite::Sprite() 
    : texture(nullptr), width(0), height(0), 
      frameCountW(1), frameCountH(1), manualFrameW(0), manualFrameH(0),
      cameraFollower(false), scale(1.0f, 1.0f), flip(SDL_FLIP_NONE) {}

Sprite::Sprite(string file, int frameCountW, int frameCountH) 
    : texture(nullptr), width(0), height(0), 
    frameCountW(frameCountW), frameCountH(frameCountH), manualFrameW(0), manualFrameH(0),
    cameraFollower(false), scale(1.0f, 1.0f), flip(SDL_FLIP_NONE) {
    Open(file);
}

Sprite::~Sprite() {}

void Sprite::SetCameraFollower(bool follower) {
    cameraFollower = follower;
}

void Sprite::Open(string file) {
    texture = Resources::GetImage(file);

    if (texture == nullptr) {
        cerr << "Erro ao carregar textura: " << SDL_GetError() << endl;
        return;
    }

    SDL_QueryTexture(texture.get(), nullptr, nullptr, &width, &height);
    SetFrame(0);
}

void Sprite::SetClip(int x, int y, int w, int h) {
    clipRect.x = x;
    clipRect.y = y;
    clipRect.w = w;
    clipRect.h = h;
}

void Sprite::Render(int x, int y, float angle) {
    Render(x, y, clipRect.w * scale.x, clipRect.h * scale.y, angle);
}

void Sprite::Render(int x, int y, int w, int h, float angle) {
    if (texture == nullptr) {
        cerr << "Erro: Textura não carregada!" << endl;
        return;
    }
    SDL_Rect destRect;
    
    if (cameraFollower) {
        destRect.x = x;
        destRect.y = y;
    } else {
        destRect.x = x - Camera::pos.x;
        destRect.y = y - Camera::pos.y;
    }
    
    destRect.w = w;
    destRect.h = h;
    SDL_RenderCopyEx(Game::GetInstance().GetRenderer(), texture.get(), &clipRect, &destRect, angle, nullptr, flip);
}

void Sprite::SetFrame(int frame) {
    // NOVO: usa o tamanho manual se foi definido via SetFrameSize(); senão,
    // cai no cálculo automático de sempre (largura_total / frameCountW).
    int frameW = (manualFrameW > 0) ? manualFrameW : width / frameCountW;
    int frameH = (manualFrameH > 0) ? manualFrameH : height / frameCountH;
    int frameX = (frame % frameCountW) * frameW;
    int frameY = (frame / frameCountW) * frameH;
    SetClip(frameX, frameY, frameW, frameH);
}

void Sprite::SetFrameCount(int frameCountW, int frameCountH) {
    this->frameCountW = frameCountW;
    this->frameCountH = frameCountH;
}

void Sprite::SetFrameSize(int frameW, int frameH) {
    manualFrameW = frameW;
    manualFrameH = frameH;
    SetFrame(0); // Reaplica o clip já com o tamanho novo
}

void Sprite::SetScale(float scaleX, float scaleY) {
    if (scaleX != 0) {
        scale.x = scaleX;
    }
    if (scaleY != 0) {
        scale.y = scaleY;
    }
}

void Sprite::SetFlip(SDL_RendererFlip flip) {
    this->flip = flip;
}

int Sprite::GetWidth() {
    int frameW = (manualFrameW > 0) ? manualFrameW : width / frameCountW;
    return frameW * scale.x;
}

int Sprite::GetHeight() {
    int frameH = (manualFrameH > 0) ? manualFrameH : height / frameCountH;
    return frameH * scale.y;
}

Vec2 Sprite::GetScale() {
    return scale;
}

bool Sprite::IsOpen() {
    return texture != nullptr;
}
