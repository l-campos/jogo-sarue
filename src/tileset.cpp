#include <iostream>
#include "tileset.h"

TileSet::TileSet(int tileWidth, int tileHeight, std::string file) 
    : tileSet(file), tileWidth(tileWidth), tileHeight(tileHeight), tileCount(0) {
    
    if (tileSet.IsOpen()) {
        int columns = tileSet.GetWidth() / tileWidth;
        int rows    = tileSet.GetHeight() / tileHeight;
        
        tileSet.SetFrameCount(columns, rows);

        // CORREÇÃO (mesma causa do bug do saruê):
        // O PNG do tileset tem 1622px de altura. 1622 ÷ 32 = 50.6875, que a
        // divisão inteira trunca pra 50 linhas. Aí o Sprite calcula
        // frameH = 1622/50 = 32.44px em vez de 32 — cada linha seguinte
        // começa 0.44px abaixo do esperado, e logo estamos desenhando pedaços
        // de tiles errados (é o "efeito fantasma" que apareceu na tela).
        // SetFrameSize força o tamanho de célula real (32x32) independente
        // do resultado da divisão.
        tileSet.SetFrameSize(tileWidth, tileHeight);

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

int TileSet::GetTileWidth()  { return tileWidth; }
int TileSet::GetTileHeight() { return tileHeight; }
