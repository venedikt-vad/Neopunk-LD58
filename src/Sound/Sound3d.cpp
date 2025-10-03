#include "Sound3d.h"

Sound3d::Sound3d() {
}

Sound3d::Sound3d(std::string path) {
    sound = LoadSound(path.c_str());
}

Sound3d::~Sound3d() {
    UnloadSound(sound);
}

void Sound3d::Play() {
    PlaySound(sound);
}

void Sound3d::Pause() {
    PauseSound(sound);
}

void Sound3d::Stop() {
    StopSound(sound);
}

bool Sound3d::IsPlayingSound() {
    return IsSoundPlaying(sound);
}

void Sound3d::SetSoundPosition(Camera listener, Vector3 position) {
    // Calculate direction vector and distance between listener and sound source
    Vector3 direction = Vector3Subtract(position, listener.position);
    float distance = Vector3Length(direction);

    // Apply logarithmic distance attenuation and clamp between 0-1
    float attenuation = 1.0f/(1.0f + (distance/maxDist));
    // float attenuation = (1/(distance/maxDist) - 0.5) * 2;
    attenuation = Clamp(attenuation, 0.0f, 1.0f);

    // Calculate normalized vectors for spatial positioning
    Vector3 normalizedDirection = Vector3Normalize(direction);
    Vector3 forward = Vector3Normalize(Vector3Subtract(listener.target, listener.position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(listener.up, forward));

    // Reduce volume for sounds behind the listener
    float dotProduct = Vector3DotProduct(forward, normalizedDirection);
    if (dotProduct < 0.0f) attenuation *= (1.0f + dotProduct*0.5f);

    // Set stereo panning based on sound position relative to listener
    float pan = 0.5f + 0.3f*Vector3DotProduct(normalizedDirection, right);

    // Apply final sound properties
    SetSoundVolume(sound, attenuation);
    SetSoundPan(sound, pan);
}

// static void SetSoundPosition(Camera listener, Sound sound, Vector3 position, float maxDist)
// {
//     // Calculate direction vector and distance between listener and sound source
//     Vector3 direction = Vector3Subtract(position, listener.position);
//     float distance = Vector3Length(direction);

//     // Apply logarithmic distance attenuation and clamp between 0-1
//     float attenuation = 1.0f/(1.0f + (distance/maxDist));
//     attenuation = Clamp(attenuation, 0.0f, 1.0f);

//     // Calculate normalized vectors for spatial positioning
//     Vector3 normalizedDirection = Vector3Normalize(direction);
//     Vector3 forward = Vector3Normalize(Vector3Subtract(listener.target, listener.position));
//     Vector3 right = Vector3Normalize(Vector3CrossProduct(listener.up, forward));

//     // Reduce volume for sounds behind the listener
//     float dotProduct = Vector3DotProduct(forward, normalizedDirection);
//     if (dotProduct < 0.0f) attenuation *= (1.0f + dotProduct*0.5f);

//     // Set stereo panning based on sound position relative to listener
//     float pan = 0.5f + 0.5f*Vector3DotProduct(normalizedDirection, right);

//     // Apply final sound properties
//     SetSoundVolume(sound, attenuation);
//     SetSoundPan(sound, pan);
// }