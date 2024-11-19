#ifndef SFX_H
#define SFX_H

#include "lib/sound.h"

enum {
    SFX_DEATH,
    SFX_DOOR,
    SFX_HURT,
    SFX_JUMP,
    SFX_PAUSE,

    SFX_NONE, // after sfx count, won't play
};

enum {
    MUSIC_BG,
};

#endif // SFX_H
