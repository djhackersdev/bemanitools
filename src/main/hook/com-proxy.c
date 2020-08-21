#include <windows.h>
#include <unknwn.h>

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "hook/com-proxy.h"

static void com_proxy_free(struct com_proxy *proxy);
static HRESULT STDMETHODCALLTYPE com_proxy_query_interface(
        IUnknown *unk,
        REFIID iid,
        void **iface);
static ULONG STDMETHODCALLTYPE com_proxy_addref(IUnknown *unk);
static ULONG STDMETHODCALLTYPE com_proxy_release(IUnknown *unk);

#ifdef __amd64

    /***** 64-BIT TRAMPOLINE *****/

#define SLOT_OFFSET 0x0A
static const uint8_t com_proxy_tramp[] = {
    /* mov rcx, [rcx+8]         ; Get this->real */
    0x48, 0x8B, 0x49, 0x08,

    /* mov rax, [rcx]           ; Get this->vtbl */
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

    /* mov ecx, [eax]           ; Get this->vtbl */
    0x8B, 0x08,

    /* mov ecx, [ecx+XX]        ; Get vtbl->slot_XX */
    0x8B, 0x89, -1, -1, -1, -1,

    /* jmp ecx                  ; Continue to slot_XX */
    0xFF, 0xE1
};

#endif

HRESULT com_proxy_wrap(
        struct com_proxy **out,
        void *real,
        size_t vtbl_size)
{
    struct com_proxy *proxy;
    void **vtbl;
    uint8_t *cur_tramp;
    size_t nslots;
    size_t i;
    HRESULT hr;

    assert(out != NULL);
    assert(real != NULL);

    *out = NULL;

    proxy = calloc(1, sizeof(*proxy));

    if (proxy == NULL) {
        hr = E_OUTOFMEMORY;

        goto end;
    }

    proxy->vptr = malloc(vtbl_size);

    if (proxy->vptr == NULL) {
        hr = E_OUTOFMEMORY;

        goto end;
    }

    nslots = vtbl_size / sizeof(void *);

    proxy->tramps = VirtualAlloc(
            NULL,
            sizeof(com_proxy_tramp) * nslots,
            MEM_RESERVE | MEM_COMMIT,
            PAGE_EXECUTE_READWRITE);

    if (proxy->tramps == NULL) {
        hr = E_OUTOFMEMORY;

        goto end;
    }

    proxy->real = real;

    /* Set up proxied IUnknown impl */

    vtbl = proxy->vptr;
    vtbl[0] = com_proxy_query_interface;
    vtbl[1] = com_proxy_addref;
    vtbl[2] = com_proxy_release;

    /* Populate trampoline code for remaining vtbl entries */

    for (i = 3 /* Skip IUnknown */ ; i < nslots ; i++) {
        cur_tramp = proxy->tramps + i * sizeof(com_proxy_tramp);

        /* Copy template */
        memcpy(cur_tramp, com_proxy_tramp, sizeof(com_proxy_tramp));

        /* Patch XX into vtbl lookup (see definition of tramp) */
        *((uint32_t *) (cur_tramp + SLOT_OFFSET)) = i * sizeof(void *);

        /* Set vtable entry */
        vtbl[i] = cur_tramp;
    }

    *out = proxy;
    proxy = NULL;
    hr = S_OK;

end:
    com_proxy_free(proxy);

    return hr;
}

static void com_proxy_free(struct com_proxy *proxy)
{
    if (proxy == NULL) {
        return;
    }

    if (proxy->cleanup_ctx != NULL) {
        proxy->cleanup_ctx(proxy->ctx);
    }

    if (proxy->tramps != NULL) {
        VirtualFree(proxy->tramps, 0, MEM_RELEASE);
    }

    free(proxy->vptr);
    free(proxy);
}

static HRESULT STDMETHODCALLTYPE com_proxy_query_interface(
        IUnknown *unk,
        REFIID iid,
        void **iface)
{
    struct com_proxy *proxy;
    IUnknown *obj;

    assert(unk != NULL);

    proxy = (struct com_proxy *) unk;
    obj = proxy->real; /* Not necessarily this object's canonical IUnknown */

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

static ULONG STDMETHODCALLTYPE com_proxy_addref(IUnknown *unk)
{
    struct com_proxy *proxy;
    IUnknown *obj;

    assert(unk != NULL);

    proxy = (struct com_proxy *) unk;
    obj = proxy->real;

    return IUnknown_AddRef(obj);
}

static ULONG STDMETHODCALLTYPE com_proxy_release(IUnknown *unk)
{
    struct com_proxy *proxy;
    IUnknown *real;
    ULONG result;

    assert(unk != NULL);

    proxy = (struct com_proxy *) unk;
    real = proxy->real;
    result = IUnknown_Release(real);

    if (!result) {
        /* Last ref to underlying object released */
        com_proxy_free(proxy);
    }

    return result;
}
