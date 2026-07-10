#include "animator.h"
#include "spriterenderer.h"
#include "gameobject.h"

Animator::Animator(GameObject& associated) 
    : Component(associated), frameStart(0), frameEnd(0), frameTime(0), 
    currentFrame(0), timeElapsed(0), isPlaying(true) {}

void Animator::Update(float dt) {
    // NOVO: Se o frameTime for 0 ou se estiver pausado, não avança a animação!
    if (frameTime <= 0 || !isPlaying) return;
    
    timeElapsed += dt;
    
    if (timeElapsed >= frameTime) {
        currentFrame++;
        timeElapsed-= frameTime;
        
        if (currentFrame > frameEnd) {
            currentFrame = frameStart;
        }
        
        SpriteRenderer* sprite = associated.GetComponent<SpriteRenderer>();
        
        if (sprite) {
            sprite->SetFrame(currentFrame);
        }
    }
}

void Animator::Render() {}

void Animator::SetAnimation(std::string name) {
    
    auto it = animations.find(name);
    
    if (it != animations.end()) {
        frameStart = it->second.frameStart;
        frameEnd = it->second.frameEnd;
        frameTime = it->second.frameTime;

        if (current != name) {
            current = name;
        } else {
            return; 
        }
        
        currentFrame = frameStart;
        timeElapsed = 0;
        isPlaying = true;
        
        SpriteRenderer* sprite = associated.GetComponent<SpriteRenderer>();
        if (sprite != nullptr) {
            sprite->SetFrame(currentFrame);
        }
    }
}

void Animator::AddAnimation(std::string name, Animation anim) {
    animations.insert({name, anim});
}