#ifdef DEBUG
#include <SDL2/SDL.h>
#include "camera.h"
#include "game.h"
#endif // DEBUG 

#include <cmath>
#include "collider.h"
#include "gameobject.h"


Collider::Collider(GameObject& associated, Vec2 scale, Vec2 offset) 
    : Component(associated), scale(scale), offset(offset) {
    Update(0);
}

void Collider::Update(float dt) {
    box.w = associated.box.w * scale.x;
    box.h = associated.box.h * scale.y;

    Vec2 center = associated.box.Center();
    
    float angleRad = associated.angleDeg * (M_PI / 180.0f);
    Vec2 rotatedOffset = offset.Rotate(angleRad);

    box.x = center.x - (box.w / 2.0f) + rotatedOffset.x;
    box.y = center.y - (box.h / 2.0f) + rotatedOffset.y;
}

void Collider::Render() {
#ifdef DEBUG
    Vec2 center( box.Center() ); 
    SDL_Point points[5];

    Vec2 point = (Vec2(box.x, box.y) - center).Rotate( associated.angleDeg/(180/M_PI) )
                    + center - Camera::pos;
    points[0] = {(int)point.x, (int)point.y};
    points[4] = {(int)point.x, (int)point.y};
    
    point = (Vec2(box.x + box.w, box.y) - center).Rotate( associated.angleDeg/(180/M_PI) )
                    + center - Camera::pos;
    points[1] = {(int)point.x, (int)point.y};
    
    point = (Vec2(box.x + box.w, box.y + box.h) - center).Rotate( associated.angleDeg/(180/M_PI) )
                    + center - Camera::pos;
    points[2] = {(int)point.x, (int)point.y};
    
    point = (Vec2(box.x, box.y + box.h) - center).Rotate( associated.angleDeg/(180/M_PI) )
                    + center - Camera::pos;
    points[3] = {(int)point.x, (int)point.y};

    SDL_SetRenderDrawColor(Game::GetInstance().GetRenderer(), 255, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLines(Game::GetInstance().GetRenderer(), points, 5);
#endif // DEBUG
}

void Collider::SetScale(Vec2 scale) {
    this->scale = scale;
}

void Collider::SetOffset(Vec2 offset) {
    this->offset = offset;
}

