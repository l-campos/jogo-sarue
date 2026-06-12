#include <iostream>
#include "tileset.h"

TileSet::TileSet(int tileWidth, int tileHeight, std::string file) 
    : tileSet(file), tileWidth(tileWidth), tileHeight(tileHeight), tileCount(0) {
    
    if (tileSet.IsOpen()) {
        int columns = tileSet.GetWidth() / tileWidth;
        int rows = tileSet.GetHeight() / tileHeight;
        
        tileSet.SetFrameCount(columns, rows);
        tileCount = columns * rows;
    } else {
        std::cerr << "Erro: Nao foi possivel carregar o TileSet: " << file << std::endl;
    }
}

void TileSet::RenderTile(unsigned index, float x, float y) {
    if (index < (unsigned)tileCount) { 
        tileSet.SetFrame(index);
        tileSet.Render(x, y);
    }
}

int TileSet::GetTileWidth() {
    return tileWidth;
}

int TileSet::GetTileHeight() {
    return tileHeight;
}