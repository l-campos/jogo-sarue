#ifndef SOUND_H
#define SOUND_H 
#define INCLUDE_SDL_MIXER

#include <string>
#include <memory>
#include "SDL_include.h"

class Sound {
public:
    Sound();
    Sound(std::string file);
    ~Sound();
    
    void Play(int times = 1);
    void Stop();
    void Open(std::string file);
    bool IsOpen();

private:
    std::shared_ptr<Mix_Chunk> chunk;
    int channel;
};

#endif