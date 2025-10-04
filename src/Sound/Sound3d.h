#pragma once

#include <map>
#include <string>
#include "raylib.h"
#include "raymath.h"

#include "SoundEffects.h"

class Sound3d {
public:
    Sound3d();
    Sound3d(std::string path, float _maxValue = 0.05f, float _maxDist = 1.f);
    ~Sound3d();

    void Play();
    void Pause();
    void Stop();
    void Resume();
    bool IsPlayingSound();
    void SetSoundPosition(Camera listener, Vector3 position);

    float maxDist = 1.f;
    float maxValue = 0.05f;

private:
    Sound sound;

    bool LPFAttached = false;
};