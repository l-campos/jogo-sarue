#ifndef TILEMAP_H
#define TILEMAP_H

#include <string>
#include <vector>
#include <memory> 
#include "component.h"
#include "tileset.h"

class TileMap : public Component {
public:
    TileMap(GameObject& associated, std::string file, TileSet* tileSet);
    
    void Load(std::string file);
    void SetTileSet(TileSet* tileSet);
    
    int& At(int x, int y, int z = 0); 
    
    void Render() override;
    void Update(float dt) override;
    void RenderLayer(int layer);
    
    int GetWidth();
    int GetHeight();
    int GetDepth();

    bool IsSolid(int gridX, int gridY);

private:
    std::vector<int> tileMatrix;
    std::unique_ptr<TileSet> tileSet; 

    int mapWidth;
    int mapHeight;
    int mapDepth;
};

#endif