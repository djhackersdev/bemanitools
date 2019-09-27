#ifndef HOOK_COM_PROXY_H
#define HOOK_COM_PROXY_H

#include <stddef.h>
#include <stdint.h>

/* N.B. Here be dragons. You really ought to be fairly familiar with COM
   before using this, otherwise you risk ignoring the subtler issues at your
   peril. */

#define COM_PROXY_UNWRAP(self) (((struct com_proxy *) self)->real)

struct com_proxy {
    /* Pointer to vtable filled with trampolines. Edit these as you please.
       Each com_proxy has its own independent vtable. */
    void *vptr;

    /* Interface pointer wrapped by this proxy. */
    void *real;

    /* Dynamically generated x86 trampoline code. The initial vtable entries
       all point into code located here. */
    uint8_t *tramps;
};

/* Wrap a COM interface pointer in a proxy. This is an object that acts just
   like the object that it wraps, but has a freely editable vtable, which you
   can modify in order to intercept a subset of the interface's method calls.

   By default, all the vtable slots contain dynamically generated trampolines
   which pass method calls onwards to the corresponding methods in the
   underlying object's vtable.

   NOTE! This does not AddRef the underlying interface.

   NOTE! This function wraps COM POINTERS, not COM OBJECTS (since the latter
   is, in general, impossible). If you're insufficiently versed in COM to
   understand the difference... well, you really should be, but the following
   observations are a start:

   1. Do not wrap IUnknown pointers with this function. This will break the
      IUnknown::QueryInterface contract. This refers to _the_ unique
      IUnknown* for the object, not for any other interface (which necessarily
      extends IUnknown). Wrapping the unique IUnknown* for an object will
      cause it to no longer be unique.

   2. Callers can "jailbreak" your wrapper using IUnknown::QueryInterface.

   Generally this isn't an issue for DirectX objects, since nobody ever seems
   to use QueryInterface with them. */

struct com_proxy *com_proxy_wrap(void *iface, size_t vtbl_size);

#endif
