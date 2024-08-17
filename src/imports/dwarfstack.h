/*
 * Copyright (C) 2013-2019 Hannes Domani
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */


#ifndef __DWARFSTACK_H__
#define __DWARFSTACK_H__

#include <stdint.h>


#if defined(DWST_STATIC)
#define EXPORT
#elif defined(DWST_SHARED)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif


#ifdef __cplusplus
extern "C" {
#endif


// dwstCallback(): callback function
//   addr:              stack address
//   filename:          source file location
//   lineno:            line number
//   funcname:          function name
//   context:           user-provided pointer (callbackContext)
//   columnno:          column number
typedef void dwstCallback(
    uint64_t addr,const char *filename,int lineno,const char *funcname,
    void *context,int columnno );

typedef void dwstCallbackW(
    uint64_t addr,const wchar_t *filename,int lineno,const char *funcname,
    void *context,int columnno );

// special values for lineno:

// DWST_BASE_ADDR: inform about the used image base address
//   (important in case it's not the same as the preferred image base address)
//   addr:              used image base address
//   filename:          executable location
#define DWST_BASE_ADDR           0

// DWST_NO_DBG_SYM: no debug information available
//   addr:              stack address
//   filename:          executable location
#define DWST_NO_DBG_SYM         -1

// DWST_NO_SRC_FILE: no source file information available
//   addr:              stack address
//   filename:          executable location
#define DWST_NO_SRC_FILE        -2

// DWST_NOT_FOUND: no information available (invalid address?)
//   addr:              stack address
//   filename:          executable location
#define DWST_NOT_FOUND          -3


// dwstOfFile(): stack information of file
//   name:              executable location
//   imageBase:         used image base address
//   addr:              stack addresses
//   count:             number of addresses
//   callbackFunc:      callback function
//   callbackContext:   user-provided pointer (context)
//      (for example see examples/addr2line/)
EXPORT int dwstOfFile(
    const char *name,uint64_t imageBase,
    uint64_t *addr,int count,
    dwstCallback *callbackFunc,void *callbackContext );

EXPORT int dwstOfFileW(
    const wchar_t *name,uint64_t imageBase,
    uint64_t *addr,int count,
    dwstCallbackW *callbackFunc,void *callbackContext );


// dwstOfProcess(): stack information of current process
//   addr:              stack addresses
//   count:             number of addresses
//   callbackFunc:      callback function
//   callbackContext:   user-provided pointer (context)
EXPORT int dwstOfProcess(
    uintptr_t *addr,int count,
    dwstCallback *callbackFunc,void *callbackContext );

EXPORT int dwstOfProcessW(
    uintptr_t *addr,int count,
    dwstCallbackW *callbackFunc,void *callbackContext );


// dwstOfLocation(): stack information of current location
//   callbackFunc:      callback function
//   callbackContext:   user-provided pointer (context)
//      (for example see examples/location/)
EXPORT int dwstOfLocation(
    dwstCallback *callbackFunc,void *callbackContext );

EXPORT int dwstOfLocationW(
    dwstCallbackW *callbackFunc,void *callbackContext );


// dwstOfException(): stack information of exception
//   context:           ContextRecord of exception
//   callbackFunc:      callback function
//   callbackContext:   user-provided pointer (context)
//      (for example see examples/exception/)
EXPORT int dwstOfException(
    void *context,
    dwstCallback *callbackFunc,void *callbackContext );

EXPORT int dwstOfExceptionW(
    void *context,
    dwstCallbackW *callbackFunc,void *callbackContext );


// dwstExceptionDialog(): show dialog on unhandled exception
//   extraInfo:         extra information shown in dialog
//      (for example see examples/exception-dialog/)
EXPORT void dwstExceptionDialog(
    const char *extraInfo );

EXPORT void dwstExceptionDialogW(
    const wchar_t *extraInfo );


#ifndef DWST_STATIC
// dwstDemangle(): demangle gcc style c++ symbols
//   mangled:           mangled name
//   demangled:         demangled name
//   length:            size of demangled buffer
EXPORT size_t dwstDemangle(
    const char *mangled,
    char *demangled,size_t length );
#endif


#ifdef __cplusplus
}
#endif

#endif
