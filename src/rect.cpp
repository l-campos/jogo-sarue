#include "rect.h"

Rect::Rect() : x(0), y(0), w(0), h(0) {}

Rect::Rect(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}

Vec2 Rect::Center() const {
    return Vec2(x + (w / 2.0f), y + (h / 2.0f));
}

float Rect::Distance(const Rect& other) const {
    return this->Center().Distance(other.Center());
}

bool Rect::Contains(const Vec2& point) const {
    return (point.x >= x && point.x <= x + w && 
            point.y >= y && point.y <= y + h);
}

Rect Rect::operator+(const Vec2& v) const {
    return Rect(x + v.x, y + v.y, w, h);
}

void Rect::operator+=(const Vec2& v) {
    x += v.x;
    y += v.y;
}