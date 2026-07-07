#include <iostream>
#include "tileset.h"

TileSet::TileSet(int tileWidth, int tileHeight, std::string file) 
    : tileSet(file), tileWidth(tileWidth), tileHeight(tileHeight), tileCount(0) {
    
    if (tileSet.IsOpen()) {
        // Calcula colunas/linhas ANTES de qualquer SetScale, enquanto GetWidth/Height
        // ainda devolvem o tamanho bruto da textura (frameCount=1, scale=1 por padrão).
        int columns = tileSet.GetWidth()  / tileWidth;   // 256 / 32 = 8
        int rows    = tileSet.GetHeight() / tileHeight;  // 1622 / 32 = 50

        tileSet.SetFrameCount(columns, rows);

        // Corrige o drift de frame: 1622/50 = 32.44px em vez de 32.
        // Mesma causa do bug que consertamos no saruê.
        tileSet.SetFrameSize(tileWidth, tileHeight);

        // Escala visual 2x: tiles passam de 32px pra 64px na tela.
        // Isso faz com que GetTileWidth/Height devolvam 64, valor usado por
        // TileMap em TODA a lógica (render, PixelToTile, TileToPixel).
        // Se precisar ajustar a escala, mude só aqui — tudo se propaga.
        tileSet.SetScale(2.0f, 2.0f);

        tileCount = columns * rows; // 8 * 50 = 400
    } else {
        std::cerr << "Erro: Nao foi possivel carregar o TileSet: " << file << std::endl;
    }
}

void TileSet::RenderTile(unsigned index, float x, float y) {
    if (index < (unsigned)tileCount) {
        tileSet.SetFrame(index);
        tileSet.Render(x, y); // tamanho final = clipRect.w * scale = 32 * 2 = 64px
    }
}

// ATENÇÃO: devolvem o tamanho RENDERIZADO (após escala), não o tamanho bruto.
// Com SetScale(2,2) e SetFrameSize(32,32): GetWidth() = 32*2 = 64.
// TileMap usa esses valores em PixelToTile e TileToPixel — se divergirem do
// tamanho visual, os blocos de colisão ficam desalinhados com os tiles na tela.
int TileSet::GetTileWidth()  { return tileSet.GetWidth(); }
int TileSet::GetTileHeight() { return tileSet.GetHeight(); }

void TileSet::SetScale(float scale) {
    tileSet.SetScale(scale, scale);
    // GetTileWidth/Height usam tileSet.GetWidth/Height() que já refletem a nova escala.
}
