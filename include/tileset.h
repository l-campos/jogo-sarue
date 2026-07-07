#ifndef TILESET_H
#define TILESET_H

#include <string>
#include "sprite.h"

class TileSet {
public:
    TileSet(int tileWidth, int tileHeight, std::string file);

    void RenderTile(unsigned index, float x, float y);

    // Muda a escala visual dos tiles. Chame ANTES de usar o TileMap, ou via
    // TileMap::SetScale() (que também atualiza os offsets do mapa automaticamente).
    // Escala padrão definida no construtor: 2.0f (tiles de 64px).
    void SetScale(float scale);

    // Devolvem o tamanho RENDERIZADO do tile (tileWidth × escala atual).
    // TileMap usa esses valores em TUDO: render, PixelToTile, TileToPixel.
    int GetTileWidth();
    int GetTileHeight();

private:
    Sprite tileSet;
    int tileWidth;
    int tileHeight;
    int tileCount;
};

#endif
