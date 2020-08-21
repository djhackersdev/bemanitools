#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>


int __cdecl CommSetSendQueFwPort(int a1, int a2)
{
    return 0;
}

int __cdecl CommInitSendWait(void)
{
    return 0;
}

int __cdecl CommInit(char * a1, int a2, int a3, int a4, int a5)
{
    return 0;
}

void __cdecl CommPolling(void)
{

}

void __cdecl CommReadBroadCast(int * a1)
{

}

void __cdecl CommReadMyAddr(int * a1)
{

}

void __cdecl CommRegistRecvFunc(int a1, void * a2, void * a3)
{
    
}

int __cdecl CommSetSendQueData(int * a1, int a2, char * a3, int a4, int a5)
{
    return 0;
}


BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    if (reason == DLL_PROCESS_ATTACH) {

    }

    return TRUE;
}
