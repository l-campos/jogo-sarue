#include <fstream>
#include <sstream>
#include <iostream>
#include "tilemap.h"
#include "gameobject.h"
#include "camera.h"
#include "json.hpp" // Biblioteca mágica para ler o JSON

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

    // Limpa a matriz antiga do jogo para não acumular lixo na memória
    tileMatrix.clear();

    // Varre todas as camadas que o designer criou no Tiled
    for (auto& layer : mapJson["layers"]) {
        // Nós só queremos camadas de blocos (tilelayer). 
        // Se o designer criou camadas de imagem ou de objetos, o código vai ignorar com segurança.
        if (layer["type"] == "tilelayer") {
            mapDepth++;
            
            // O vetor "data" contém todos os IDs de cada quadradinho do mapa
            for (int tileID : layer["data"]) {
                // REGRA CRÍTICA DO TILED: 
                // No Tiled, o pixel vazio é salvo como 0, e os blocos começam em 1, 2, 3...
                // Na arquitetura da sua disciplina, o vazio costuma ser -1 e os blocos começam em 0, 1, 2...
                // Por isso, subtraímos 1 do ID que vem do arquivo para casar perfeitamente!
                tileMatrix.push_back(tileID - 1);
            }
        }
    }

    printf("Mapa carregado com Sucesso! Dimensoes: %dx%d | Camadas: %d\n", mapWidth, mapHeight, mapDepth);
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
    // Puxamos as dimensões dos blocos do nosso TileSet (normalmente 32x32)
    int tileW = tileSet->GetTileWidth();
    int tileH = tileSet->GetTileHeight();

    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            int index = At(x, y, layer);

            if (index >= 0) {
                // CORREÇÃO CRÍTICA: Subtraímos APENAS a Camera::pos, sem multiplicar pela camada!
                // O std::round ajuda a evitar "tremidelas" nos blocos se a câmara usar números decimais.
                float renderX = (x * tileW) - std::round(Camera::pos.x);
                float renderY = (y * tileH) - std::round(Camera::pos.y);

                tileSet->RenderTile(index, renderX, renderY);
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
    // Se o personagem tentar sair do limite do mapa, bate em uma parede invisível
    if (gridX < 0 || gridX >= mapWidth || gridY < 0 || gridY >= mapHeight) return true; 
    
    // A Camada 0 (z=0) é a camada das Paredes/Chão.
    int index = At(gridX, gridY, 0); 
    
    // Se tiver qualquer bloco desenhado ali (index >= 0), é sólido!
    return (index >= 0);
}