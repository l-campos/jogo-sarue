#ifndef TILEMAP_H
#define TILEMAP_H

#include <string>
#include <vector>
#include <memory> 
#include "component.h"
#include "tileset.h"

class TileMap : public Component {
public:
    TileMap(GameObject& associated, std::string tmjFile, TileSet* tileSet);
    
    void Load(std::string tmjFile);
    void SetTileSet(TileSet* tileSet);
    
    void Render() override;
    void Update(float dt) override;
    void RenderLayer(const std::vector<int>& layer);
    
    int GetWidth();
    int GetHeight();
    int GetTileWidth();
    int GetTileHeight();

    // Consultas de gameplay em coordenadas de TILE (coluna/linha)
    bool IsSolidTile(int tileX, int tileY);
    bool IsClimbTile(int tileX, int tileY);
    bool IsWaterTile(int tileX, int tileY);

    // Consultas em coordenadas de PIXEL do mundo (o que os outros componentes usam)
    bool IsSolidAtPixel(float worldX, float worldY);
    bool IsClimbAtPixel(float worldX, float worldY);
    bool IsWaterAtPixel(float worldX, float worldY);

    // Conversao pixel <-> indice de tile, POR EIXO, descontando o offset.
    // Antes era um metodo unico "PixelToTile(float pixel)" que nao sabia
    // se estava convertendo X ou Y -- com offsets diferentes por eixo isso
    // dava resultado errado. Agora ha uma versao para cada eixo.
    int   PixelToTileX(float worldX);
    int   PixelToTileY(float worldY);
    float TileToPixelX(int tileX);
    float TileToPixelY(int tileY);

    void SetScale(float scale); // atualiza escala do TileSet E recomputa offsets

    // Ponteiro estatico pro TileMap ativo (mesmo padrao de Character::player / Camera)
    static TileMap* instance;

private:
    std::vector<int> groundLayer;
    std::vector<int> climbLayer;
    std::vector<int> waterLayer;

    std::unique_ptr<TileSet> tileSet;

    int   mapWidth;
    int   mapHeight;

    // Offset escalado (world-space): usado em render e colisao.
    float mapOffsetX;
    float mapOffsetY;

    // Offset bruto do JSON (coordenadas de 32px do Tiled). Guardado pra que
    // SetScale() possa recomputar mapOffsetX/Y ao mudar a escala sem precisar
    // reler o arquivo .tmj.
    float rawOffsetX;
    float rawOffsetY;

    int At(const std::vector<int>& layer, int x, int y);
};

#endif
