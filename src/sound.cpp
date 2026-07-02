#include <iostream>
#include "sound.h"
#include "resources.h"

Sound::Sound() : chunk(nullptr), channel(-1) {}

Sound::Sound(std::string file) : Sound() {
    Open(file);
}

Sound::~Sound() {}

void Sound::Play(int times) {
    if (chunk != nullptr) {
        channel = Mix_PlayChannel(-1, chunk.get(), times - 1);
    }
}

void Sound::Stop() {
    if (chunk != nullptr && channel != -1) {
        Mix_HaltChannel(channel);
        channel = -1;
    }
}

void Sound::Open(std::string file) {
    chunk = Resources::GetSound(file);
    if (chunk == nullptr) {
        std::cerr << "Erro ao carregar o som: " << SDL_GetError() << std::endl;
    }
}

bool Sound::IsOpen() {
    return chunk != nullptr;
}
