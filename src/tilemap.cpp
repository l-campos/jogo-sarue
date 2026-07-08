#include <fstream>
#include <sstream>
#include <iostream>
#include "tilemap.h"
#include "gameobject.h"
#include "camera.h"
#include "json.hpp" // Biblioteca para ler o JSON
#include "game.h"

using json = nlohmann::json;

TileMap::TileMap(GameObject& associated, std::string file, TileSet* tileSet) 
    : Component(associated) {
    Load(file);
    SetTileSet(tileSet);
}

void TileMap::Load(std::string file) {
    std::ifstream f(file);
    
    if (!f.is_open()) {
        printf("Erro Crítico: Nao foi possivel abrir o arquivo de mapa JSON em: %s\n", file.c_str());
        return;
    }

    // O json.hpp faz o parse do arquivo inteiro automaticamente
    json mapJson;
    try {
        f >> mapJson;
    } catch (json::parse_error& e) {
        printf("Erro ao ler a sintaxe do JSON: %s\n", e.what());
        return;
    }

    // Resgata a largura e a altura do mapa (em quantidade de blocos)
    mapWidth = mapJson["width"];
    mapHeight = mapJson["height"];
    
    // Zera a profundidade (camadas) antes de contar
    mapDepth = 0;

    // Limpa a matriz e o mapa
    tileMatrix.clear();
    layerMap.clear();

    // Varre todas as camadas que o designer criou no Tiled
    for (auto& layer : mapJson["layers"]) {
        if (layer["type"] == "tilelayer") {
            
            // Salva o nome da camada apontando para o Z atual
            std::string layerName = layer.value("name", "unnamed");
            layerMap[layerName] = mapDepth;

            // O vetor "data" contém todos os IDs de cada quadradinho do mapa
            for (int tileID : layer["data"]) {
                // REGRA CRÍTICA DO TILED: -1 para alinhar com o nosso índice
                tileMatrix.push_back(tileID - 1);
            }
            
            mapDepth++;
        }
    }

    printf("Mapa carregado com Sucesso! Dimensoes: %dx%d | Camadas: %d\n", mapWidth, mapHeight, mapDepth);
}

void TileMap::SetTileSet(TileSet* tileSet) {
    this->tileSet.reset(tileSet);
}

int& TileMap::At(int x, int y, int z) {
    int depth = (z * mapWidth * mapHeight);
    int height = (y * mapWidth);
    return tileMatrix[depth + height + x];
}

void TileMap::RenderLayer(int layer) {
    int tileW = tileSet->GetTileWidth();
    int tileH = tileSet->GetTileHeight();

    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            int index = At(x, y, layer);

            if (index >= 0) {
                // Posição PURA do bloco no mundo (sem subtrair a câmera)
                float worldX = x * tileW;
                float worldY = y * tileH;

                // O Sprite::Render já subtrai a câmera automaticamente lá dentro!
                tileSet->RenderTile(index, worldX, worldY);

                // --- INÍCIO DO BLOCO DE DEBUG ---
#ifdef DEBUG
                // Como o SDL_RenderDrawRect pinta os quadrados de depuração direto 
                // na tela, AQUI NÓS PRECISAMOS subtrair a câmera manualmente.
                SDL_Rect tileRect = {(int)(worldX - Camera::pos.x), 
                                     (int)(worldY - Camera::pos.y), 
                                     tileW, tileH};
                
                SDL_SetRenderDrawColor(Game::GetInstance().GetRenderer(), 0, 255, 0, SDL_ALPHA_OPAQUE);
                SDL_RenderDrawRect(Game::GetInstance().GetRenderer(), &tileRect);
#endif
                // --- FIM DO BLOCO DE DEBUG ---
            }
        }
    }
}

void TileMap::Render() {
    for (int layer = 0; layer < mapDepth; layer++) {
        RenderLayer(layer);
    }
}

int TileMap::GetWidth() {
    return mapWidth;
}

int TileMap::GetHeight() { 
    return mapHeight; 
}

int TileMap::GetDepth() { 
    return mapDepth; 
}

void TileMap::Update(float dt) {}

bool TileMap::IsSolid(int gridX, int gridY) {
    // 1. Paredes invisíveis nas laterais (não deixa fugir do mapa pela esquerda/direita)
    if (gridX < 0 || gridX >= mapWidth) return true;

    // 2. Teto livre: Se pular acima do mapa, é "ar" (permite subir em plataformas altas)
    if (gridY < 0) return false;

    // 3. Fundo livre: Se cair abaixo do mapa, é "ar" (permite cair e morrer no abismo)
    if (gridY >= mapHeight) return false;

    // Procura pela camada PLATAFORMA. Se ela existir no JSON e tiver um bloco desenhado ali, é sólido.
    if (layerMap.find("PLATAFORMA") != layerMap.end()) {
        int z = layerMap["PLATAFORMA"];
        if (At(gridX, gridY, z) >= 0) {
            return true;
        }
    }

    if (layerMap.find("PLATAFORMA ARVORE") != layerMap.end()) {
        int z = layerMap["PLATAFORMA ARVORE"];
        if (At(gridX, gridY, z) >= 0) {
            return true;
        }
    }
    
    return false;
}

bool TileMap::IsWater(int gridX, int gridY) {
    if (gridX < 0 || gridX >= mapWidth || gridY < 0 || gridY >= mapHeight) return false;

    if (layerMap.find("ÁGUA") != layerMap.end()) {
        int z = layerMap["ÁGUA"];
        if (At(gridX, gridY, z) >= 0) {
            return true;
        }
    }
    return false;
}

bool TileMap::IsClimbing(int gridX, int gridY) {
    if (gridX < 0 || gridX >= mapWidth || gridY < 0 || gridY >= mapHeight) return false;

    if (layerMap.find("CANOS ESCALAR") != layerMap.end()) {
        int z = layerMap["CANOS ESCALAR"];
        if (At(gridX, gridY, z) >= 0) {
            return true;
        }
    }
    return false;
}