#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <unordered_map>
#include <string>
#include "component.h"
#include "animation.h"

class Animator : public Component {
public:
    Animator(GameObject& associated);

    void Update(float dt) override;
    void Render() override;
    void SetAnimation(std::string name);
    void AddAnimation(std::string name, Animation anim);

    std::string current;

private:
    std::unordered_map<std::string, Animation> animations;

    int frameStart;
    int frameEnd;
    float frameTime;
    
    int currentFrame;
    float timeElapsed;
};

#endif