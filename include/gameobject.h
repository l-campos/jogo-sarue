#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <vector>
#include "rect.h"

class Component;

class GameObject {
public:
    GameObject();
    ~GameObject();
    
    void Start();
    void Update(float dt);
    void Render();
    void RequestDelete();
    void AddComponent(Component* cpt);
    void RemoveComponent(Component* cpt);
    void NotifyCollision(GameObject& other);
    
    bool IsDead() const;
    
    template <typename T>
    T* GetComponent(){
	    long unsigned int index;
	    for(index=0;index<components.size();index++){
		    T* component = dynamic_cast<T*>(components[index]);
		    if(component != nullptr){
			    return component;
		    }
	    }
        return nullptr;
    }
    
    Rect box;
    double angleDeg;
    
private:
    bool isDead;
    bool started;
    std::vector<Component*> components;
};

#endif