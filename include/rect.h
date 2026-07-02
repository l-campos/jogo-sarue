#ifndef RECT_H
#define RECT_H

#include "vec2.h" 

class Rect {
public:
    float x, y, w, h; 
    Rect(); 
    Rect(float x, float y, float w, float h); 
    Vec2 Center() const; 
    float Distance(const Rect& other) const; 
    bool Contains(const Vec2& point) const; 
    Rect operator+(const Vec2& v) const;     
    void operator+=(const Vec2& v); 
};

#endif