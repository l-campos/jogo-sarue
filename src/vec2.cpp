#include <cmath> 
#include "vec2.h"

Vec2::Vec2() : x(0), y(0) {}

Vec2::Vec2(float x, float y) : x(x), y(y) {}

Vec2 Vec2::operator+(const Vec2& other) const {
    return Vec2(x + other.x, y + other.y); 
}

Vec2 Vec2::operator-(const Vec2& other) const {
    return Vec2(x - other.x, y - other.y);
}

Vec2 Vec2::operator*(const float scalar) const {
    return Vec2(x * scalar, y * scalar);
}

float Vec2::Magnitude() const {
    return std::sqrt(x * x + y * y);
}

Vec2 Vec2::Normalize() const {
    float mag = Magnitude();
    if (mag == 0) return Vec2(0, 0); 
    return Vec2(x / mag, y / mag);
}

float Vec2::Distance(const Vec2& other) const {
    return (*this - other).Magnitude(); 
}

float Vec2::Inclination() const {
    return std::atan2(y, x);
}

float Vec2::Inclination(const Vec2& other) const {
    return (other - *this).Inclination();
}

Vec2 Vec2::Rotate(float angle) const {
    float x_prime = x * std::cos(angle) - y * std::sin(angle); 
    float y_prime = y * std::cos(angle) + x * std::sin(angle); 
    return Vec2(x_prime, y_prime);
}