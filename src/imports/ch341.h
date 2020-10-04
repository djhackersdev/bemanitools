#ifndef IMPORTS_CH341_H
#define IMPORTS_CH341_H

// The bare minimum definitions to implement jbio-magicbox
// There is a full .h for this DLL floating around online, if you care

HANDLE WINAPI CH341OpenDevice(ULONG iIndex);
VOID WINAPI CH341CloseDevice(ULONG iIndex);
BOOL WINAPI CH341EppReadData(ULONG iIndex, PVOID oBuffer, PULONG ioLength);
BOOL WINAPI CH341EppSetAddr(ULONG iIndex, UCHAR iAddr);

#endif
