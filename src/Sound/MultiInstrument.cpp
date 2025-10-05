#include "MultiInstrument.h"

#include <initializer_list>

MultiInstrument::MultiInstrument() {}

MultiInstrument::MultiInstrument(std::initializer_list < std::string > paths) {
    for (const auto & path: paths) {
        Sound sound = LoadSound(path.c_str());
        SetSoundVolume(sound, 0.7f);
        sounds.insert({
            path,
            sound
        });
    }
}

MultiInstrument::~MultiInstrument() {
    for (const auto & pair: sounds) {
        UnloadSound(pair.second);
    }
    sounds.clear();
}

void MultiInstrument::Update() {
    if (!isPlaying) return;

    if (currentSoundKey.empty() || !IsSoundPlaying(sounds[currentSoundKey])) {
        PlayOnce();
    }
}

void MultiInstrument::PlayOnce() {
    if (sounds.empty()) return;

    int index = std::rand() % sounds.size();
    auto it = sounds.begin();
    std::advance(it, index);

    PlaySound(it->second);
    currentSoundKey = it->first;
}

void MultiInstrument::Play() {
    isPlaying = true;
}

void MultiInstrument::Stop() {
    isPlaying = false;
}