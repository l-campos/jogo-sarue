#ifndef CHARACTER_H
#define CHARACTER_H

#include <queue>
#include <memory>
#include <string>
#include "component.h"
#include "timer.h"
#include "vec2.h"


class Character : public Component {
public:
    Character(GameObject& associated, std::string sprite);
    ~Character();

    void Start() override;
    void Update(float dt) override;
    void Render() override;

    class Command {
    public:
        enum CommandType { MOVE, JUMP }; // Inserir ATTACK ao implementar ataque
        Command(CommandType type, float x, float y) : type(type), pos(x, y) {}    
        CommandType type;
        Vec2 pos;
    };

    void Issue(Command task);
    static Character* player;
    Vec2 GetPosition();
    
    private:
    std::weak_ptr<GameObject> gun;
    std::queue<Command> taskQueue;
    
    Vec2 speed;
    int hp;
    float linearSpeed;
    void NotifyCollision(GameObject& other) override;
    
    Timer deathTimer;
    Timer damageCooldown;
    
    bool isGrounded;
    const float GRAVITY = 1500.0f;     
    const float JUMP_FORCE = 600.0f;   
    const float MOVE_SPEED = 300.0f;   
    const float MAX_FALL_SPEED = 800.0f; 
};

#endif