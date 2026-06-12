#include <iostream>
#include "resources.h"
#include "game.h"

std::unordered_map<std::string, std::shared_ptr<SDL_Texture>> Resources::imageTable;
std::unordered_map<std::string, std::shared_ptr<Mix_Music>> Resources::musicTable;
std::unordered_map<std::string, std::shared_ptr<Mix_Chunk>> Resources::soundTable;
std::unordered_map<std::string, std::shared_ptr<TTF_Font>> Resources::fontTable;

std::shared_ptr<SDL_Texture> Resources::GetImage(std::string file) {
    auto it = imageTable.find(file);
    if (it != imageTable.end()) {
        return it->second;
    }

    SDL_Texture* texture = IMG_LoadTexture(Game::GetInstance().GetRenderer(), file.c_str());
    if (texture == nullptr) {
        std::cerr << "Erro ao carregar imagem: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    // Criando o shared_ptr com a função lambda de destruição
    std::shared_ptr<SDL_Texture> sharedTex(texture, [](SDL_Texture* t) { SDL_DestroyTexture(t); });
    imageTable.emplace(file, sharedTex);
    
    return sharedTex;
}

void Resources::ClearImages() {
    for (auto it = imageTable.begin(); it != imageTable.end(); ) {
        if (it->second.use_count() == 1) { // Só a Resources conhece? Então apaga!
            it = imageTable.erase(it);
        } else {
            it++;
        }
    }
}

std::shared_ptr<Mix_Music> Resources::GetMusic(std::string file) {
    auto it = musicTable.find(file);
    if (it != musicTable.end()) {
        return it->second;
    }

    Mix_Music* music = Mix_LoadMUS(file.c_str());
    if (music == nullptr) {
        std::cerr << "Erro ao carregar musica: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    std::shared_ptr<Mix_Music> sharedMusic(music, [](Mix_Music* m) { Mix_FreeMusic(m); });
    musicTable.emplace(file, sharedMusic);
    
    return sharedMusic;
}

void Resources::ClearMusics() {
    for (auto it = musicTable.begin(); it != musicTable.end(); ) {
        if (it->second.use_count() == 1) {
            it = musicTable.erase(it);
        } else {
            it++;
        }
    }
}

std::shared_ptr<Mix_Chunk> Resources::GetSound(std::string file) {
    auto it = soundTable.find(file);
    if (it != soundTable.end()) {
        return it->second;
    }

    Mix_Chunk* chunk = Mix_LoadWAV(file.c_str());
    if (chunk == nullptr) {
        std::cerr << "Erro ao carregar som: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    std::shared_ptr<Mix_Chunk> sharedChunk(chunk, [](Mix_Chunk* c) { Mix_FreeChunk(c); });
    soundTable.emplace(file, sharedChunk);
    
    return sharedChunk;
}

void Resources::ClearSounds() {
    for (auto it = soundTable.begin(); it != soundTable.end(); ) {
        if (it->second.use_count() == 1) {
            it = soundTable.erase(it);
        } else {
            it++;
        }
    }
}

std::shared_ptr<TTF_Font> Resources::GetFont(std::string file, int fontSize) {
    std::string key = file + std::to_string(fontSize);
    auto it = fontTable.find(key);
    
    if (it != fontTable.end()) {
        return it->second;
    }
    
    TTF_Font* font = TTF_OpenFont(file.c_str(), fontSize);
    if (font == nullptr) {
        std::cerr << "Erro ao carregar fonte: " << file << " - " << SDL_GetError() << std::endl;
        return nullptr;
    }
    
    std::shared_ptr<TTF_Font> sharedFont(font, [](TTF_Font* f) { TTF_CloseFont(f); });
    fontTable.emplace(key, sharedFont);
    return sharedFont;
}

void Resources::ClearFonts() {
    for (auto it = fontTable.begin(); it != fontTable.end(); ) {
        if (it->second.use_count() == 1) {
            it = fontTable.erase(it);
        } else {
            it++;
        }
    }
}