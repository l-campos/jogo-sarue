#ifndef ENEMY_H
#define ENEMY_H

#include "component.h"
#include "vec2.h"
#include "timer.h"
#include "tilemap.h"

class Enemy : public Component {
public:
    Enemy(GameObject& associated, float startX, float startY, TileMap* map);
    
    void Update(float dt) override;
    void Render() override;
    void NotifyCollision(GameObject& other) override;

    // Função de receber dano do Saruê
    void Damage(int damage, Vec2 attackerPos);

private:
    enum EnemyState { PATROL, DIVE, EXPLODE, RETURNING };
    EnemyState state;
    
    Vec2 speed;
    float startX; // Ponto inicial da patrulha
    float startY; // A altura que ele gosta de voar

    // NOVO: posição travada no instante em que ele avista o jogador e começa
    // o mergulho. Depois disso ele NÃO reage mais a onde o jogador vai (nem
    // se pular) — só continua até esse ponto fixo.
    Vec2 diveTarget;

    // NOVO: pequena janela entre "chegou no alvo" e o delete de verdade, útil
    // se no futuro quiserem tocar uma animação/efeito de explosão antes de sumir.
    Timer explodeTimer;

    // Sistema de atordoamento (Stun e HP)
    int hp;
    Timer damageCooldown;
    bool isStunned;

    TileMap* map;
};

#endif
