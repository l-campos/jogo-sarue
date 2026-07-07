#include "spriterenderer.h"
#include "gameobject.h"

SpriteRenderer::SpriteRenderer(GameObject& associated) : Component(associated) {}

SpriteRenderer::SpriteRenderer(GameObject& associated, std::string file, int frameCountW, int frameCountH) 
    : Component(associated), sprite(file, frameCountW, frameCountH) {
    associated.box.w = sprite.GetWidth();
    associated.box.h = sprite.GetHeight();
    sprite.SetFrame(0);
}

Vec2 SpriteRenderer::GetScale() {
    return sprite.GetScale();
}

void SpriteRenderer::SetFrameCount(int frameCountW, int frameCountH) {
    sprite.SetFrameCount(frameCountW, frameCountH);
}

void SpriteRenderer::SetFrameSize(int frameW, int frameH) {
    sprite.SetFrameSize(frameW, frameH);
    // Recalcula a caixa de colisão/renderização com o tamanho de célula certo
    // (senão o Collider continua usando a largura/altura erradas calculadas
    // automaticamente no construtor).
    associated.box.w = sprite.GetWidth();
    associated.box.h = sprite.GetHeight();
}

void SpriteRenderer::SetCameraFollower(bool follower) {
    sprite.SetCameraFollower(follower);
}

void SpriteRenderer::SetFrame(int frame) {
    sprite.SetFrame(frame);
}

void SpriteRenderer::SetScale(float scaleX, float scaleY) {
    sprite.SetScale(scaleX, scaleY);
    associated.box.w = sprite.GetWidth();
    associated.box.h = sprite.GetHeight();
}

void SpriteRenderer::SetFlip(SDL_RendererFlip flip) {
    sprite.SetFlip(flip);
}

void SpriteRenderer::Open(std::string file) {
    sprite.Open(file);
    associated.box.w = sprite.GetWidth();
    associated.box.h = sprite.GetHeight();
}

void SpriteRenderer::Update(float dt) {}

void SpriteRenderer::Render() {
    sprite.Render(associated.box.x, associated.box.y, associated.box.w, associated.box.h, associated.angleDeg);
}
