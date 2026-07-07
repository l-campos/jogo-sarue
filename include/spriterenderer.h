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

    // NOVO: repassa pro Sprite interno. Use quando a largura/altura total da
    // spritesheet não divide certinho por frameCountW/frameCountH (ex.: sobrou
    // espaço extra pra um frame que extrapola o grid).
    void SetFrameSize(int frameW, int frameH);

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
