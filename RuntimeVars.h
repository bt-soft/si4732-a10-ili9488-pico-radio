#ifndef __RUNTIMEVARS_H
#define __RUNTIMEVARS_H

#include <sys/types.h>

// AGC
extern uint8_t currentAGCgain;
extern uint8_t currentAGCgainStep;

// BFO
extern bool bfoOn;

// Mute
#define AUDIO_MUTE_ON true
#define AUDIO_MUTE_OFF false
extern bool muteStat;

// Squelch
#define SQUELCH_DECAY_TIME 500
#define MIN_SQUELCH 0
#define MAX_SQUELCH 50
extern long squelchDecay;

// Scan
extern bool SCANpause;

// Seek
extern bool SEEK;

#endif
