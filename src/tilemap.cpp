#include <fstream>
#include <sstream>
#include <iostream>
#include "tilemap.h"
#include "gameobject.h"
#include "camera.h"

TileMap::TileMap(GameObject& associated, std::string file, TileSet* tileSet) 
    : Component(associated) {
    Load(file);
    SetTileSet(tileSet);
}

void TileMap::Load(std::string file) {
    std::ifstream f(file);
    
    if (f.is_open()) {
        std::string item;
        
        if (std::getline(f, item, ',')) mapWidth = std::stoi(item);
        if (std::getline(f, item, ',')) mapHeight = std::stoi(item);
        if (std::getline(f, item, ',')) mapDepth = std::stoi(item);
        
        while (std::getline(f, item, ',')) {
            try {
                tileMatrix.push_back(std::stoi(item)); 
            } catch (...) {}
        }
        f.close();
    } else {
        std::cerr << "Erro: Nao foi possivel abrir o arquivo: " << file << std::endl;
    }
}
void TileMap::SetTileSet(TileSet* tileSet) {
    this->tileSet.reset(tileSet);
}

int& TileMap::At(int x, int y, int z) {
    int depth = (z * mapWidth * mapHeight) ;
    int height = (y * mapWidth);
    return tileMatrix[depth + height + x];
}

void TileMap::RenderLayer(int layer) {
    if (!tileSet) return; 
    
    int tileW = tileSet->GetTileWidth();
    int tileH = tileSet->GetTileHeight();

    float parallaxFactor = 1.0f + (layer * 0.1f);
    
    for (int y = 0; y < mapHeight; y++) {
        for (int x = 0; x < mapWidth; x++) {
            
            int tileIndex = At(x, y, layer);
            
            int actualTile = tileIndex; 
            
            if (actualTile >= 0) {
                float posX = (x * tileW) + associated.box.x - Camera::pos.x * parallaxFactor; 
                float posY = (y * tileH) + associated.box.y - Camera::pos.y * parallaxFactor; 
                
                tileSet->RenderTile((unsigned)actualTile, posX, posY);
            }
        }
    }
}

void TileMap::Render() {
    for (int layer = 0; layer < mapDepth; layer++) {
        RenderLayer(layer);
    }
}

int TileMap::GetWidth() { return mapWidth; }
int TileMap::GetHeight() { return mapHeight; }
int TileMap::GetDepth() { return mapDepth; }

void TileMap::Update(float dt) {}