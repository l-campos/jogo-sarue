#ifndef CHARACTER_H
#define CHARACTER_H

#include <queue>
#include <string>
#include "component.h"
#include "timer.h"
#include "vec2.h"

class Character : public Component {
public:
    class Command {
    public:
        enum CommandType { MOVE, JUMP, PLAY_DEAD, ATTACK, DASH }; 
        Command(CommandType type, float x, float y) : type(type), pos(x, y) {}    
        CommandType type;
        Vec2 pos;
    };

    Character(GameObject& associated, std::string sprite);
    ~Character();

    void Start() override;
    void Update(float dt) override;
    void Render() override;
    void NotifyCollision(GameObject& other) override;
    
    void Issue(Command task);
    Vec2 GetPosition();
    bool IsPlayingDead();
    int GetHP();    

    static Character* player;

private:
    std::queue<Command> taskQueue;
    
    Vec2 speed;
    int hp;
    float linearSpeed;
    
    Timer deathTimer;
    Timer damageCooldown;
    Timer dashTimer;
    Timer dashCooldown;
    Timer attackTimer;
    
    bool isGrounded;
    bool isPlayingDead;
    bool isDashing;
    bool isFacingLeft;
    bool isAttacking;
    bool isBagging;
    bool isScaling; // <- Voltou para permitir escalada
    bool isHanging;
    bool isDying;
};

#endif