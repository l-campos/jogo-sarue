#include "gameobject.h"
#include "component.h"

GameObject::GameObject() : angleDeg(0), isDead(false), started(false)  {}

GameObject::~GameObject() {
    for (auto it = components.begin(); it != components.end(); it++) {
        delete *it;
    }
}

void GameObject::Start() {
    for (auto it = components.begin(); it != components.end(); it++) {
        (*it)->Start();
    }
    started = true;
}

void GameObject::Update(float dt) {
    for (auto it = components.begin(); it != components.end(); it++) {
        (*it)->Update(dt);
    }
}

void GameObject::Render() {
    for (auto it = components.begin(); it != components.end(); it++) {
        (*it)->Render();
    }
}

void GameObject::NotifyCollision(GameObject& other) {
    for (auto it = components.begin(); it != components.end(); it++) {
        (*it)->NotifyCollision(other);
    }
}

void GameObject::RequestDelete() {
    isDead = true;
}

void GameObject::AddComponent(Component* cpt) {
    components.push_back(cpt);
    
    if (started) {
        cpt->Start();
    }
}

void GameObject::RemoveComponent(Component* cpt) {
    for (auto it = components.begin(); it != components.end(); it++) {
        if (*it == cpt) {
            components.erase(it);
            break; 
        }
    }
}

bool GameObject::IsDead() const {
    return isDead;
}