#include <fstream>
#include <iostream>
#include "tilemap.h"
#include "gameobject.h"
#include "camera.h"
#include "json.hpp" // nlohmann/json — single header, https://github.com/nlohmann/json
#include "game.h"

using json = nlohmann::json;

TileMap* TileMap::instance = nullptr;

TileMap::TileMap(GameObject& associated, std::string tmjFile, TileSet* tileSet)
    : Component(associated), mapWidth(0), mapHeight(0),
      mapOffsetX(0.0f), mapOffsetY(0.0f), rawOffsetX(0.0f), rawOffsetY(0.0f) {
    Load(tmjFile);
    SetTileSet(tileSet);
    instance = this;
}

void TileMap::Load(std::string tmjFile) {
    std::ifstream f(tmjFile);
    if (!f.is_open()) {
        std::cerr << "Erro: Nao foi possivel abrir: " << tmjFile << std::endl;
        return;
    }

    json d;
    try { f >> d; }
    catch (json::parse_error& e) {
        std::cerr << "Erro no JSON do mapa: " << e.what() << std::endl;
        return;
    }

    mapWidth  = d.value("width",  0);
    mapHeight = d.value("height", 0);

    groundLayer.assign(mapWidth * mapHeight, 0);
    climbLayer .assign(mapWidth * mapHeight, 0);
    waterLayer .assign(mapWidth * mapHeight, 0);

    // Escala do tile (deve bater com o SetScale feito em tileset.cpp).
    // O offset exportado pelo Tiled está em coordenadas de 32px (tile nativo).
    // Multiplicamos pelo mesmo fator de escala para que offsets de render e
    // de colisão fiquem no mesmo espaço de coordenadas.
    const float TILE_SCALE = 2.0f;

    bool offsetRead = false;

    for (auto& layer : d["layers"]) {
        if (layer.value("type", "") != "tilelayer") continue;
        if (!layer.contains("data")) continue;

        if (!offsetRead) {
            rawOffsetX = layer.value("offsetx", 0.0f);
            rawOffsetY = layer.value("offsety", 0.0f);
            mapOffsetX = rawOffsetX * TILE_SCALE;
            mapOffsetY = rawOffsetY * TILE_SCALE;
            offsetRead = true;
        }

        std::string name = layer.value("name", "");
        std::vector<int>* target = nullptr;

        if      (name.find("Blocos")  != std::string::npos) target = &groundLayer;
        else if (name.find("escalar") != std::string::npos) target = &climbLayer;
        else if (name.find("gua")     != std::string::npos) target = &waterLayer; // "água"
        else continue;

        int i = 0;
        for (auto& gidVal : layer["data"]) {
            int gid = gidVal.get<int>();
            if (i < (int)target->size() && gid != 0)
                (*target)[i] = gid;
            i++;
        }
    }
}

void TileMap::SetTileSet(TileSet* ts) { tileSet.reset(ts); }

void TileMap::SetScale(float scale) {
    if (!tileSet) return;

    // 1. Atualiza a escala visual do TileSet (tamanho renderizado dos tiles)
    tileSet->SetScale(scale);

    // 2. Recomputa os offsets no mesmo sistema de coordenadas.
    //    rawOffsetX/Y guardam os valores brutos do Tiled (coordenadas de 32px).
    //    O offset escalado = raw * (tamanho_renderizado / tamanho_original_32px).
    //    Como GetTileWidth() já reflete a nova escala: fator = GetTileWidth() / 32.
    float factor = tileSet->GetTileWidth() / 32.0f;
    mapOffsetX = rawOffsetX * factor;
    mapOffsetY = rawOffsetY * factor;
}

int TileMap::At(const std::vector<int>& layer, int x, int y) {
    if (x < 0 || y < 0 || x >= mapWidth || y >= mapHeight) return 0;
    return layer[y * mapWidth + x];
}

// ─── Pixel  ↔  Tile ────────────────────────────────────────────────────────
//
// tileSet->GetTileWidth/Height() devolve o tamanho RENDERIZADO (64px se scale=2).
// mapOffsetX/Y está no mesmo sistema (já foi multiplicado por TILE_SCALE no Load).
//
// Regra:  tileX = (worldX - mapOffsetX) / tileRenderW
//         worldX = tileX * tileRenderW + mapOffsetX
//
// Por que subtrair mapOffset?
//   O tile de coluna 0 não começa em worldX=0 — começa em worldX=mapOffsetX.
//   Sem subtrair, worldX=500 seria mapeado para o tile errado (deslocado).
//
// Por que dividir por tileRenderW (64) e não por tileWidth (32)?
//   Porque os tiles ocupam 64px de espaço no mundo. Se dividíssemos por 32,
//   um pixel dentro do tile visual 3 seria mapeado para o tile 6 do grid —
//   exatamente os "buracos" que apareciam no mapa escalado.

int TileMap::PixelToTileX(float worldX) {
    int tileW = tileSet ? tileSet->GetTileWidth() : 32;
    return (int)((worldX - mapOffsetX) / tileW);
}
int TileMap::PixelToTileY(float worldY) {
    int tileH = tileSet ? tileSet->GetTileHeight() : 32;
    return (int)((worldY - mapOffsetY) / tileH);
}
float TileMap::TileToPixelX(int tileX) {
    int tileW = tileSet ? tileSet->GetTileWidth() : 32;
    return (float)(tileX * tileW) + mapOffsetX;
}
float TileMap::TileToPixelY(int tileY) {
    int tileH = tileSet ? tileSet->GetTileHeight() : 32;
    return (float)(tileY * tileH) + mapOffsetY;
}

// ─── Consultas de tile ──────────────────────────────────────────────────────

bool TileMap::IsSolidTile(int tx, int ty) { return At(groundLayer, tx, ty) != 0; }
bool TileMap::IsClimbTile(int tx, int ty) { return At(climbLayer,  tx, ty) != 0; }
bool TileMap::IsWaterTile(int tx, int ty) { return At(waterLayer,  tx, ty) != 0; }

bool TileMap::IsSolidAtPixel(float wx, float wy) { return IsSolidTile(PixelToTileX(wx), PixelToTileY(wy)); }
bool TileMap::IsClimbAtPixel(float wx, float wy) { return IsClimbTile(PixelToTileX(wx), PixelToTileY(wy)); }
bool TileMap::IsWaterAtPixel(float wx, float wy) { return IsWaterTile(PixelToTileX(wx), PixelToTileY(wy)); }

// ─── Render ─────────────────────────────────────────────────────────────────

void TileMap::RenderLayer(const std::vector<int>& layer) {
    if (!tileSet) return;

    int tileW = tileSet->GetTileWidth();   // 64 (rendered size)
    int tileH = tileSet->GetTileHeight();  // 64

    for (int y = 0; y < mapHeight; y++) {
        for (int x = 0; x < mapWidth; x++) {
            int gid = At(layer, x, y);
            if (gid <= 0) continue;

            int localIndex = gid - 1; // Tiled usa firstgid=1; índice local é 0-based

            // CORREÇÃO DA DUPLA SUBTRAÇÃO:
            // NÃO subtraímos Camera::pos aqui. Sprite::Render já subtrai internamente
            // (cameraFollower = false por padrão). Se subtraíssemos aqui também,
            // o tile se deslocaria 2× mais rápido que o personagem quando a câmera
            // se move, fazendo o mapa "sumir" durante o movimento.
            float posX = (x * tileW) + mapOffsetX + associated.box.x;
            float posY = (y * tileH) + mapOffsetY + associated.box.y;

            tileSet->RenderTile((unsigned)localIndex, posX, posY);

            #ifdef DEBUG
                // Como o SDL_RenderDrawRect pinta os quadrados de depuração direto 
                // na tela (não passa pela classe Sprite), AQUI NÓS PRECISAMOS 
                // subtrair a câmera manualmente para as linhas verdes baterem com as imagens.
                SDL_Rect tileRect = {(int)(posX - Camera::pos.x), 
                                     (int)(posY - Camera::pos.y), 
                                     tileW, tileH};
                
                SDL_SetRenderDrawColor(Game::GetInstance().GetRenderer(), 0, 255, 0, SDL_ALPHA_OPAQUE);
                SDL_RenderDrawRect(Game::GetInstance().GetRenderer(), &tileRect);
#endif
                // --- FIM DO BLOCO DE DEBUG ---
            }
        }
    }

void TileMap::Render() {
    RenderLayer(groundLayer);
    RenderLayer(climbLayer);
    RenderLayer(waterLayer);
}

int TileMap::GetWidth()      { return mapWidth; }
int TileMap::GetHeight()     { return mapHeight; }
int TileMap::GetTileWidth()  { return tileSet ? tileSet->GetTileWidth()  : 32; }
int TileMap::GetTileHeight() { return tileSet ? tileSet->GetTileHeight() : 32; }

void TileMap::Update(float dt) {}
