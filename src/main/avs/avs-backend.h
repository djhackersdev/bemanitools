#ifndef AVS_BACKEND_H
#define AVS_BACKEND_H

// TODO have implementations of all avs functions in the
// .c file that call the vtable ptr
void avs_backend_init(const struct avs_vfptr *vtable);

#endif