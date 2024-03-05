#ifndef AVS_LOADER_H
#define AVS_LOADER_H

struct avs_vfptr {
    struct avs_version {
        uint16_t major;
        uint16_t minor;
        uint16_t patch;
    } version;
  
    // TODO have all avs functions in here as one giant vtable
};

// TODO add to hook API to init it with the vtable ptr
// loader just takes the functions available staticaly and populates it
// to allow it being passed to other libraries in the same process
// and ensuring every process is using the same AVS implementation
void avs_loader_load(struct avs_vfptr *vtable);

// TODO maybe split up the AVS library into thread, log, property, etc?
// see also that logging can switch before AVS is initialized etc problem

#endif