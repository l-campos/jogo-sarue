#include "fruit.h"
#include "spriterenderer.h"
#include "collider.h"

Fruit::Fruit(GameObject& associated, float x, float y) : Component(associated) {
    associated.box.x = x;
    associated.box.y = y;

    // Use uma imagem provisória "Fruit.png" ou qualquer outra que você tenha
    SpriteRenderer* sprite = new SpriteRenderer(associated, "img/Bullet.png");
    associated.AddComponent(sprite);

    // O colisor para o Saruê poder encostar
    Collider* collider = new Collider(associated);
    associated.AddComponent(collider);
}

void Fruit::Update(float dt) {
    // Atualiza o colisor para acompanhar a caixa do GameObject
    Collider* collider = associated.GetComponent<Collider>();
    if (collider != nullptr) {
        collider->Update(dt);
    }

    // Se o objeto ficar 500 pixels para trás da câmera, deleta-o para salvar memória!
    if (associated.box.x < Camera::pos.x - 500.0f) {
        associated.RequestDelete();
    }
}

void Fruit::Render() {}

void Fruit::NotifyCollision(GameObject& other) {}