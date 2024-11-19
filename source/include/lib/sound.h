#ifndef SOUND_H
#define SOUND_H

#include "types.h"

// play sounds on different channels
void __fastcall__ sound_play_ch0(u8 sound);
void __fastcall__ sound_play_ch1(u8 sound);

void __fastcall__ music_play(u8 song);
void __fastcall__ music_stop(void);

// should be called after music_play
void music_resume(void);
void music_pause(void);

void __fastcall__ music_save(u8 song);
void music_restore(void);

#endif // SOUND_H
