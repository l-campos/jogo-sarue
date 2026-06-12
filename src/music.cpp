#include <iostream>
#include "music.h"
#include "resources.h"

using namespace std;

Music::Music() : music(nullptr) {}

Music::Music(string file) : music(nullptr) {
    Open(file);
}

Music::~Music() {}

void Music::Play(int times) {
    if (music == nullptr) {
        cerr << "Erro: Música não carregada!" << endl;
        return;
    }
    Mix_PlayMusic(music.get(), times);
}

void Music::Stop(int msToStop) {
    Mix_FadeOutMusic(msToStop);
}

void Music::Open(string file) {
    music = Resources::GetMusic(file);
    if (music == nullptr) {
        cerr << "Erro ao carregar música: " << Mix_GetError() << endl;
    }
}

bool Music::IsOpen() {
    return music != nullptr;
}   