#ifndef POPNHOOK_UTIL_MIXER_H
#define POPNHOOK_UTIL_MIXER_H

/**
 * Hook some mixer related functions and stub out attempts to change the volume.
 * In addition to being annoying, this can fail on Vista+ on some outputs,
 * despite not actually preventing sound from playing.
 */
void popnhook_mixer_hook_init(void);

#endif
