#include "RuntimeVars.h"

// AGC
uint8_t currentAGCgain = 1;
uint8_t currentAGCgainStep = 1;

// BFO
bool bfoOn = false;

// Mute
#define AUDIO_MUTE_ON true
#define AUDIO_MUTE_OFF false
bool muteStat = false;

// Squelch
long squelchDecay = 0;

// Scan
bool SCANpause = true; // LWH - SCANpause must be initialized to a value else the squelch function will

// Seek
bool SEEK = false;