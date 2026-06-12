#ifndef STAGESTATE_H
#define STAGESTATE_H

#include <memory>
#include <vector>
#include "sprite.h"
#include "music.h"
#include "gameobject.h"
#include "tilemap.h"
#include "state.h"

class StageState : public State {
public:
    StageState();
    ~StageState();

    void LoadAssets() override;
    void Update(float dt) override;
    void Render() override;

    void Start() override;
    void Pause() override;
    void Resume() override;
    
private:
    TileSet* tileSet;
    TileMap* tileMap;
    Music backgroundMusic;
    Music music;    
};

#endif