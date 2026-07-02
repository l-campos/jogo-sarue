#include <fstream>
#include <iostream>
#include "tilemap.h"
#include "gameobject.h"
#include "camera.h"
#include "json.hpp" // nlohmann/json - single header, https://github.com/nlohmann/json

using json = nlohmann::json;

TileMap* TileMap::instance = nullptr;

TileMap::TileMap(GameObject& associated, std::string tmjFile, TileSet* tileSet) 
    : Component(associated), mapWidth(0), mapHeight(0), mapOffsetX(0.0f), mapOffsetY(0.0f) {
    Load(tmjFile);
    SetTileSet(tileSet);
    instance = this;
}

void TileMap::Load(std::string tmjFile) {
    std::ifstream f(tmjFile);
    if (!f.is_open()) {
        std::cerr << "Erro: Nao foi possivel abrir o arquivo: " << tmjFile << std::endl;
        return;
    }

    json d;
    try {
        f >> d;
    } catch (json::parse_error& e) {
        std::cerr << "Erro ao interpretar JSON do mapa (" << tmjFile << "): " << e.what() << std::endl;
        return;
    }

    mapWidth  = d.value("width",  0);
    mapHeight = d.value("height", 0);

    groundLayer.assign(mapWidth * mapHeight, 0);
    climbLayer .assign(mapWidth * mapHeight, 0);
    waterLayer .assign(mapWidth * mapHeight, 0);

    bool offsetRead = false;

    for (auto& layer : d["layers"]) {
        if (layer.value("type", "") != "tilelayer") continue;
        if (!layer.contains("data")) continue;

        // CORRECAO BUG 2: le o offset de posicao exportado pelo Tiled.
        // Todas as camadas tem o mesmo offset nesse mapa, entao basta ler
        // uma vez. Esse valor representa onde o grid de tiles comeca no
        // espaco do mundo (em pixels) -- precisa ser aplicado tanto no
        // render quanto nas consultas de colisao.
        if (!offsetRead) {
            mapOffsetX = layer.value("offsetx", 0.0f);
            mapOffsetY = layer.value("offsety", 0.0f);
            offsetRead = true;
        }

        std::string name = layer.value("name", "");

        std::vector<int>* target = nullptr;
        if (name.find("Blocos") != std::string::npos) {
            target = &groundLayer;
        } else if (name.find("escalar") != std::string::npos) {
            target = &climbLayer;
        } else if (name.find("gua") != std::string::npos) {
            // "gua" em vez de "agua" para nao depender de acentuacao/encoding
            target = &waterLayer;
        } else {
            continue;
        }

        int i = 0;
        for (auto& gidValue : layer["data"]) {
            int gid = gidValue.get<int>();
            if (i < (int)target->size() && gid != 0) {
                (*target)[i] = gid;
            }
            i++;
        }
    }
}

void TileMap::SetTileSet(TileSet* ts) {
    tileSet.reset(ts);
}

int TileMap::At(const std::vector<int>& layer, int x, int y) {
    if (x < 0 || y < 0 || x >= mapWidth || y >= mapHeight) return 0;
    return layer[y * mapWidth + x];
}

// ---------------------------------------------------------------------------
// Conversoes pixel <-> tile com offset
// ---------------------------------------------------------------------------
// CORRECAO BUG 3: subtrair o offset antes de dividir pelo tamanho do tile.
// Sem isso, um pixel em x=500 vira o tile 500/32=15, quando o tile correto
// e (500 - 192.667) / 32 = 9 -- a colisao consultava um tile completamente
// diferente do que estava sendo desenhado na tela.

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

// ---------------------------------------------------------------------------
// Consultas de tile
// ---------------------------------------------------------------------------

bool TileMap::IsSolidTile(int tx, int ty) { return At(groundLayer, tx, ty) != 0; }
bool TileMap::IsClimbTile(int tx, int ty) { return At(climbLayer,  tx, ty) != 0; }
bool TileMap::IsWaterTile(int tx, int ty) { return At(waterLayer,  tx, ty) != 0; }

bool TileMap::IsSolidAtPixel(float wx, float wy) {
    return IsSolidTile(PixelToTileX(wx), PixelToTileY(wy));
}
bool TileMap::IsClimbAtPixel(float wx, float wy) {
    return IsClimbTile(PixelToTileX(wx), PixelToTileY(wy));
}
bool TileMap::IsWaterAtPixel(float wx, float wy) {
    return IsWaterTile(PixelToTileX(wx), PixelToTileY(wy));
}

// ---------------------------------------------------------------------------
// Render
// ---------------------------------------------------------------------------

void TileMap::RenderLayer(const std::vector<int>& layer) {
    if (!tileSet) return;

    int tileW = tileSet->GetTileWidth();
    int tileH = tileSet->GetTileHeight();

    for (int y = 0; y < mapHeight; y++) {
        for (int x = 0; x < mapWidth; x++) {
            int gid = At(layer, x, y);
            if (gid <= 0) continue;

            // gid - firstgid (= 1) da o indice local 0-based para o TileSet
            int localIndex = gid - 1;

            // CORRECAO BUG 2: adiciona mapOffsetX/Y ao calculo de posicao,
            // para que os tiles sejam desenhados onde o Tiled os posicionou.
            float posX = (x * tileW) + mapOffsetX + associated.box.x - Camera::pos.x;
            float posY = (y * tileH) + mapOffsetY + associated.box.y - Camera::pos.y;

            tileSet->RenderTile((unsigned)localIndex, posX, posY);
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
