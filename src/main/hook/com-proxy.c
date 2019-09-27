#include <windows.h>
#include <unknwn.h>

#include <stdlib.h>

#include "hook/com-proxy.h"

#include "util/defs.h"
#include "util/mem.h"

#ifdef _WIN64

    /***** 64-BIT TRAMPOLINE *****/

#define SLOT_OFFSET 0x0A
static const uint8_t com_proxy_tramp[] = {
    /* mov rcx, [rcx+8]         ; Replace this with this->real */
    0x48, 0x8B, 0x49, 0x08,

    /* mov rax, [rcx]           ; Get real->vtbl */
    0x48, 0x8B, 0x01,

    /* mov rax, [rax+XX]        ; Get vtbl->slot_XX */
    0x48, 0x8B, 0x80, -1, -1, -1, -1,

    /* jmp rax                  ; Continue to slot_XX */
    0xFF, 0xE0,
};

#else

    /***** 32-BIT TRAMPOLINE *****/

#define SLOT_OFFSET 0x0F
static const uint8_t com_proxy_tramp[] = {
    /* mov eax, [esp+4]         ; Get this */
    0x8B, 0x44, 0x24, 0x04,

    /* mov eax, [eax+4]         ; Get this->real */
    0x8B, 0x40, 0x04,

    /* mov [esp+4], eax         ; Replace this with this->real on stack */
    0x89, 0x44, 0x24, 0x04,

    /* mov ecx, [eax]           ; Get real->vtbl */
    0x8B, 0x08,

    /* mov ecx, [ecx+XX]        ; Get vtbl->slot_XX */
    0x8B, 0x89, -1, -1, -1, -1,

    /* jmp ecx                  ; Continue to slot_XX */
    0xFF, 0xE1
};

#endif

static HRESULT STDCALL com_proxy_query_interface(IUnknown *ptr, REFIID iid,
    void **iface)
{
    struct com_proxy *self = (struct com_proxy *) ptr;
    IUnknown *obj = self->real; /* Not necessarily the real IUnknown* */

    /* To some extent, COM is designed to support shennanigans like these.
       We can safely pass the call straight through to the underlying
       interface pointer because of the following:

       "It is specifically not the case that queries for interfaces other
       than IUnknown (even the same interface through the same pointer)
       must return the same pointer value."

                           - MSDN documentation for IUnknown::QueryInterface()

       Of course, pretty much everyone screws up COM's conventions (probably
       including me in this very module to be honest), so if someone ends up
       relying on broken assumptions then this could well get a lot more
       complicated. */

    return IUnknown_QueryInterface(obj, iid, iface);
}

static ULONG STDCALL com_proxy_addref(IUnknown *ptr)
{
    struct com_proxy *self = (struct com_proxy *) ptr;
    IUnknown *obj = self->real;

    return IUnknown_AddRef(obj);
}

static ULONG STDCALL com_proxy_release(IUnknown *ptr)
{
    struct com_proxy *self = (struct com_proxy *) ptr;
    IUnknown *obj = self->real;
    ULONG result;

    result = IUnknown_Release(obj);

    if (!result) {
        /* Last ref to underlying object released */
        VirtualFree(self->tramps, 0, MEM_RELEASE);
        free(self->vptr);
        free(self);
    }

    return result;
}

struct com_proxy *com_proxy_wrap(void *iface, size_t vtbl_size)
{
    struct com_proxy *self;
    void **vtbl;
    uint8_t *cur_tramp;
    uint32_t nslots;
    uint32_t i;

    nslots = vtbl_size / sizeof(void *);

    self = xmalloc(sizeof(*self));
    self->vptr = xmalloc(vtbl_size);
    self->real = iface;
    self->tramps = VirtualAlloc(NULL, sizeof(com_proxy_tramp) * nslots,
            MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    /* Set up proxied IUnknown impl */
    vtbl = self->vptr;
    vtbl[0] = com_proxy_query_interface;
    vtbl[1] = com_proxy_addref;
    vtbl[2] = com_proxy_release;

    /* Populate trampoline code for remaining vtbl entries */
    for (i = 3 /* Skip IUnknown */ ; i < nslots ; i++) {
        cur_tramp = self->tramps + i * sizeof(com_proxy_tramp);

        /* Copy template */
        memcpy(cur_tramp, com_proxy_tramp, sizeof(com_proxy_tramp));

        /* Patch XX into vtbl lookup (see definition of tramp) */
        *((uint32_t *) (cur_tramp + SLOT_OFFSET)) = i * sizeof(void *);

        /* Set vtable entry */
        vtbl[i] = cur_tramp;
    }

    return self;
}

