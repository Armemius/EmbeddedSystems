#ifndef MUSIC_H
#define MUSIC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void play_note(uint16_t freq);

void stop_note();

void play_amogus();

#ifdef __cplusplus
}
#endif

#endif // MUSIC_H
