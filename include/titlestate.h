#ifndef TITLESTATE_H
#define TITLESTATE_H

#include "state.h"
#include "timer.h"
#include "music.h"

class TitleState : public State {
public:
    TitleState();
    ~TitleState();

    void LoadAssets() override;
    void Update(float dt) override;
    void Render() override;

    void Start() override;
    void Pause() override;
    void Resume() override;

private:
    Timer textTimer;
    bool showText;
    GameObject* textObject;
    Music backgroundMusic;
};

#endif