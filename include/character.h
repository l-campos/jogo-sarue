#ifndef CHARACTER_H
#define CHARACTER_H

#include <queue>
#include <memory>
#include <string>
#include "component.h"
#include "timer.h"
#include "vec2.h"
#include "tilemap.h"

class Character : public Component {
public:
    Character(GameObject& associated, std::string sprite, TileMap* map = nullptr);
    ~Character();

    void Start() override;
    void Update(float dt) override;
    void Render() override;

    class Command {
    public:
        enum CommandType { MOVE, JUMP, PLAY_DEAD, ATTACK, DASH, DROP_DOWN }; 
        Command(CommandType type, float x, float y) : type(type), pos(x, y) {}    
        CommandType type;
        Vec2 pos;
    };
    
    void Issue(Command task);
    static Character* player;
    Vec2 GetPosition();
    
    bool IsPlayingDead();
    int GetHP();    

private:
    std::weak_ptr<GameObject> gun;
    std::queue<Command> taskQueue;
    
    Vec2 speed;
    int hp;
    float linearSpeed;
    void NotifyCollision(GameObject& other) override;
    
    // Timers
    Timer deathTimer;
    Timer damageCooldown;
    Timer dashTimer;
    Timer dashCooldown;
    Timer attackTimer;
    Timer dropTimer;
    
    // Flags de estado
    bool isGrounded;
    bool isPlayingDead;
    bool isDashing;
    bool isFacingLeft;
    bool isAttacking;
    bool isScaling;
    bool isHanging;
    bool isDying;
    bool droppingDown;

    TileMap* map;
};

#endif