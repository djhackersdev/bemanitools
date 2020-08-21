#pragma once

#include <stddef.h>
#include <stdint.h>

#define com_proxy_downcast(self) ((struct com_proxy *) self)

struct com_proxy {
    /* Pointer to vtable filled with trampolines. Edit these as you please.
       Each com_proxy has its own independent vtable. */
    void *vptr;

    /* Interface pointer wrapped by this proxy. */
    void *real;

    /* Context pointer for use by hook code. */
    void *ctx;

    /* Optional cleanup callback, will be called during com_proxy deallocation
       to clean up *ctx. */
    void (*cleanup_ctx)(void *ctx);

    /* Dynamically generated x86 trampoline code. The initial vtable entries
       all point into code located here. */
    uint8_t *tramps;
};

/* Wrap a COM interface pointer in a proxy. This is an object that acts just
   like the object that it wraps but has a freely editable vtable, which you
   can modify in order to intercept a subset of the interface's method calls.

   By default, all the vtable slots contain dynamically generated trampolines
   which pass method calls onwards to the corresponding methods in the
   underlying object's vtable.

   NOTE! This does not AddRef the underlying interface.

   NOTE! This function wraps COM POINTERS, not COM OBJECTS (since the latter
   is, in general, impossible). Consequences of this distinction include the
   following:

   1. Do not wrap IUnknown pointers with this function. This will break the
      IUnknown::QueryInterface contract. This refers to _the_ unique
      IUnknown* for the object, not for any other interface (which necessarily
      extends IUnknown). Wrapping the unique IUnknown* for an object will
      cause it to no longer be unique.

   2. Callers can "jailbreak" your wrapper using IUnknown::QueryInterface
      unless you provide a custom QueryInterface implementation to prevent them
      from doing so. */

HRESULT com_proxy_wrap(
        struct com_proxy **out,
        void *real,
        size_t vtbl_size);
