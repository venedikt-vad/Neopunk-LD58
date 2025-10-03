#pragma once

#include <map>
#include <string>
#include "raylib.h"

class MultiInstrument {
public:
    MultiInstrument();
    MultiInstrument(std::initializer_list<std::string> paths);
    ~MultiInstrument();

    void Update();
    void PlayOnce();
    void Play();
    void Stop();

private:
    std::map<std::string, Sound> sounds;
    bool isPlaying = false;

    std::string currentSoundKey;
};