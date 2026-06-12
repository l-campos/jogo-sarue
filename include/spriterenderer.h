#ifndef SPRITERENDERER_H
#define SPRITERENDERER_H

#include "component.h"
#include "sprite.h"

class SpriteRenderer : public Component {
public:
    SpriteRenderer(GameObject& associated);
    SpriteRenderer(GameObject& associated, std::string file, int frameCountW = 1, int frameCountH = 1);
    
    void Open(std::string file);
    void SetFrameCount(int frameCountW, int frameCountH);
    void SetFrame(int frame);
    void SetCameraFollower(bool follower);
    
    void SetScale(float scaleX, float scaleY);
    Vec2 GetScale();
    void SetFlip(SDL_RendererFlip flip);
    
    void Update(float dt) override;
    void Render() override;

private:
    Sprite sprite;
};

#endif