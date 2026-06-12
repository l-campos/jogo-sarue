#ifndef SPRITE_H
#define SPRITE_H
#define INCLUDE_SDL

#include <string>
#include <memory>
#include "SDL_include.h"
#include "camera.h"

class Sprite {
public:
    Sprite();
    Sprite(std::string file, int frameCountW = 1, int frameCountH = 1);
    ~Sprite();

    void Open(std::string file);
    void SetClip(int x, int y, int w, int h);
    void Render(int x, int y, float angle = 0);    
    void Render(int x, int y, int w, int h, float angle = 0);
    
    void SetFrame(int frame);
    void SetFrameCount(int frameCountW, int frameCountH);
    void SetScale(float scaleX, float scaleY);
    Vec2 GetScale();
    void SetFlip(SDL_RendererFlip flip);
    
    int GetWidth();
    int GetHeight();
    bool IsOpen();

    void SetCameraFollower(bool follower); 

private:
    std::shared_ptr<SDL_Texture> texture;
    
    int width; 
    int height;
    int frameCountW;
    int frameCountH;
    
    bool cameraFollower; 
    
    SDL_Rect clipRect;
    Vec2 scale;
    SDL_RendererFlip flip;
};

#endif