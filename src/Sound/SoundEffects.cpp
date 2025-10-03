#include "SoundEffects.h"
#include <math.h>


void AudioProcessEffectLPF(void* buffer, unsigned int frames) {
    static float lowL = 0.0f, lowR = 0.0f;

    const float sampleRate = 44100.0f;
    const float cutoffHz = 750.0f;            // was 70 Hz -> much less “heavy”
    const float a = 1.0f - expf(-2.0f * (float)PI * cutoffHz / sampleRate);
    const float wet = 0.8f; // 0..1; lower = lighter effect


    float* x = (float*)buffer;
    for (unsigned int i = 0; i < frames * 2; i += 2) {
        const float l = x[i], r = x[i + 1];
        lowL += a * (l - lowL);
        lowR += a * (r - lowR);
        x[i] = wet * lowL + (1.0f - wet) * l;
        x[i + 1] = wet * lowR + (1.0f - wet) * r;
        //x[i] = lowL;
        //x[i + 1] = lowR;
    }
}
