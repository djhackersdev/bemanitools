#include "dinput/device_dinput8.h"

static ULONG REF_COUNT = 0;

static HRESULT STDMETHODCALLTYPE IDirectInputDevice8W_QueryInterface(
    IDirectInputDevice8W FAR *This, REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL) {
        return E_POINTER;
    }

    if (IsEqualGUID(riid, &IID_IUnknown) ||
        IsEqualGUID(riid, &IID_IDirectInputDeviceA) ||
        IsEqualGUID(riid, &IID_IDirectInputDeviceW) ||
        IsEqualGUID(riid, &IID_IDirectInputDevice2A) ||
        IsEqualGUID(riid, &IID_IDirectInputDevice2W) ||
        IsEqualGUID(riid, &IID_IDirectInputDevice7A) ||
        IsEqualGUID(riid, &IID_IDirectInputDevice7W) ||
        IsEqualGUID(riid, &IID_IDirectInputDevice8A) ||
        IsEqualGUID(riid, &IID_IDirectInputDevice8W)) {
        This->lpVtbl->AddRef(This);
        *ppvObj = This;

        return S_OK;
    }

    return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE
IDirectInputDevice8W_AddRef(IDirectInputDevice8W FAR *This)
{
    return ++REF_COUNT;
}

static ULONG STDMETHODCALLTYPE
IDirectInputDevice8W_Release(IDirectInputDevice8W FAR *This)
{
    return --REF_COUNT;
}

static HRESULT STDMETHODCALLTYPE IDirectInputDevice8W_GetCapabilities(
    IDirectInputDevice8W FAR *This, LPDIDEVCAPS lpDIDevCaps)
{
    return DIERR_INVALIDPARAM;
}

static HRESULT STDMETHODCALLTYPE IDirectInputDevice8W_EnumObjects(
    IDirectInputDevice8W FAR *This,
    LPDIENUMDEVICEOBJECTSCALLBACKW lpCallback,
    LPVOID pvRef,
    DWORD dwFlags)
{
    return DIERR_INVALIDPARAM;
}

static HRESULT STDMETHODCALLTYPE IDirectInputDevice8W_GetProperty(
    IDirectInputDevice8W FAR *This, REFGUID rguidProp, LPDIPROPHEADER pdiph)
{
    return DIERR_INVALIDPARAM;
}

static HRESULT STDMETHODCALLTYPE IDirectInputDevice8W_SetProperty(
    IDirectInputDevice8W FAR *This, REFGUID rguidProp, LPCDIPROPHEADER pdiph)
{
    return DI_OK;
}

static HRESULT STDMETHODCALLTYPE
IDirectInputDevice8W_Acquire(IDirectInputDevice8W FAR *This)
{
    return DIERR_INVALIDPARAM;
}

static HRESULT STDMETHODCALLTYPE
IDirectInputDevice8W_Unacquire(IDirectInputDevice8W FAR *This)
{
    return DIERR_INVALIDPARAM;
}

static HRESULT STDMETHODCALLTYPE IDirectInputDevice8W_GetDeviceState(
    IDirectInputDevice8W FAR *This, DWORD cbData, LPVOID lpvData)
{
    return DIERR_INVALIDPARAM;
}

static HRESULT STDMETHODCALLTYPE IDirectInputDevice8W_GetDeviceData(
    IDirectInputDevice8W FAR *This,
    DWORD cbObjectData,
    LPDIDEVICEOBJECTDATA rgdod,
    LPDWORD pdwInOut,
    DWORD dwFlags)
{
    return DIERR_INVALIDPARAM;
}

static HRESULT STDMETHODCALLTYPE IDirectInputDevice8W_SetDataFormat(
    IDirectInputDevice8W FAR *This, LPCDIDATAFORMAT lpdf)
{
    return DI_OK;
}

static HRESULT STDMETHODCALLTYPE IDirectInputDevice8W_SetEventNotification(
    IDirectInputDevice8W FAR *This, HANDLE hEvent)
{
    return DIERR_INVALIDPARAM;
}

static HRESULT STDMETHODCALLTYPE IDirectInputDevice8W_SetCooperativeLevel(
    IDirectInputDevice8W FAR *This, HWND hWnd, DWORD dwFlags)
{
    return DI_OK;
}

static HRESULT STDMETHODCALLTYPE IDirectInputDevice8W_GetObjectInfo(
    IDirectInputDevice8W FAR *This,
    LPDIDEVICEOBJECTINSTANCEW pdidoi,
    DWORD dwObj,
    DWORD dwHow)
{
    return DIERR_INVALIDPARAM;
}

static HRESULT STDMETHODCALLTYPE IDirectInputDevice8W_GetDeviceInfo(
    IDirectInputDevice8W FAR *This, LPDIDEVICEINSTANCEW pdidi)
{
    return DIERR_INVALIDPARAM;
}

static HRESULT STDMETHODCALLTYPE IDirectInputDevice8W_RunControlPanel(
    IDirectInputDevice8W FAR *This, HWND hwndOwner, DWORD dwFlags)
{
    return DIERR_INVALIDPARAM;
}

static HRESULT STDMETHODCALLTYPE IDirectInputDevice8W_Initialize(
    IDirectInputDevice8W FAR *This,
    HINSTANCE hinst,
    DWORD dwVersion,
    REFGUID rguid)
{
    return DIERR_INVALIDPARAM;
}

static HRESULT STDMETHODCALLTYPE IDirectInputDevice8W_CreateEffect(
    IDirectInputDevice8W FAR *This,
    REFGUID rguid,
    LPCDIEFFECT lpeff,
    LPDIRECTINPUTEFFECT *ppdeff,
    LPUNKNOWN punkOuter)
{
    return DIERR_INVALIDPARAM;
}

static HRESULT STDMETHODCALLTYPE IDirectInputDevice8W_EnumEffects(
    IDirectInputDevice8W FAR *This,
    LPDIENUMEFFECTSCALLBACKW lpCallback,
    LPVOID pvRef,
    DWORD dwEffType)
{
    return DIERR_INVALIDPARAM;
}

static HRESULT STDMETHODCALLTYPE IDirectInputDevice8W_GetEffectInfo(
    IDirectInputDevice8W FAR *This, LPDIEFFECTINFOW pdei, REFGUID rguid)
{
    return DIERR_INVALIDPARAM;
}

static HRESULT STDMETHODCALLTYPE IDirectInputDevice8W_GetForceFeedbackState(
    IDirectInputDevice8W FAR *This, LPDWORD pdwOut)
{
    return DIERR_INVALIDPARAM;
}

static HRESULT STDMETHODCALLTYPE IDirectInputDevice8W_SendForceFeedbackCommand(
    IDirectInputDevice8W FAR *This, DWORD dwFlags)
{
    return DIERR_INVALIDPARAM;
}

static HRESULT STDMETHODCALLTYPE IDirectInputDevice8W_EnumCreatedEffectObjects(
    IDirectInputDevice8W FAR *This,
    LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback,
    LPVOID pvRef,
    DWORD fl)
{
    return DIERR_INVALIDPARAM;
}

static HRESULT STDMETHODCALLTYPE
IDirectInputDevice8W_Escape(IDirectInputDevice8W FAR *This, LPDIEFFESCAPE pesc)
{
    return DIERR_INVALIDPARAM;
}

static HRESULT STDMETHODCALLTYPE
IDirectInputDevice8W_Poll(IDirectInputDevice8W FAR *This)
{
    return DIERR_INVALIDPARAM;
}

static HRESULT STDMETHODCALLTYPE IDirectInputDevice8W_SendDeviceData(
    IDirectInputDevice8W FAR *This,
    DWORD cbObjectData,
    LPCDIDEVICEOBJECTDATA rgdod,
    LPDWORD pdwInOut,
    DWORD fl)
{
    return DIERR_INVALIDPARAM;
}

static HRESULT STDMETHODCALLTYPE IDirectInputDevice8W_EnumEffectsInFile(
    IDirectInputDevice8W FAR *This,
    LPCWSTR lpszFileName,
    LPDIENUMEFFECTSINFILECALLBACK pec,
    LPVOID pvRef,
    DWORD dwFlags)
{
    return DIERR_INVALIDPARAM;
}

static HRESULT STDMETHODCALLTYPE IDirectInputDevice8W_WriteEffectToFile(
    IDirectInputDevice8W FAR *This,
    LPCWSTR lpszFileName,
    DWORD dwEntries,
    LPDIFILEEFFECT rgDiFileEft,
    DWORD dwFlags)
{
    return DIERR_INVALIDPARAM;
}

static HRESULT STDMETHODCALLTYPE IDirectInputDevice8W_BuildActionMap(
    IDirectInputDevice8W FAR *This,
    LPDIACTIONFORMATW lpdiaf,
    LPCWSTR lpszUserName,
    DWORD dwFlags)
{
    return DIERR_INVALIDPARAM;
}

static HRESULT STDMETHODCALLTYPE IDirectInputDevice8W_SetActionMap(
    IDirectInputDevice8W FAR *This,
    LPDIACTIONFORMATW lpdiaf,
    LPCWSTR lpszUserName,
    DWORD dwFlags)
{
    return DIERR_INVALIDPARAM;
}

static HRESULT STDMETHODCALLTYPE IDirectInputDevice8W_GetImageInfo(
    IDirectInputDevice8W FAR *This,
    LPDIDEVICEIMAGEINFOHEADERW lpdiDevImageInfoHeader)
{
    return DIERR_INVALIDPARAM;
}

static IDirectInputDevice8WVtbl stub_device_vtbl = {

    // IUnknown
    .QueryInterface = IDirectInputDevice8W_QueryInterface,
    .AddRef = IDirectInputDevice8W_AddRef,
    .Release = IDirectInputDevice8W_Release,

    // IDirectInputDeviceW
    .GetCapabilities = IDirectInputDevice8W_GetCapabilities,
    .EnumObjects = IDirectInputDevice8W_EnumObjects,
    .GetProperty = IDirectInputDevice8W_GetProperty,
    .SetProperty = IDirectInputDevice8W_SetProperty,
    .Acquire = IDirectInputDevice8W_Acquire,
    .Unacquire = IDirectInputDevice8W_Unacquire,
    .GetDeviceState = IDirectInputDevice8W_GetDeviceState,
    .GetDeviceData = IDirectInputDevice8W_GetDeviceData,
    .SetDataFormat = IDirectInputDevice8W_SetDataFormat,
    .SetEventNotification = IDirectInputDevice8W_SetEventNotification,
    .SetCooperativeLevel = IDirectInputDevice8W_SetCooperativeLevel,
    .GetObjectInfo = IDirectInputDevice8W_GetObjectInfo,
    .GetDeviceInfo = IDirectInputDevice8W_GetDeviceInfo,
    .RunControlPanel = IDirectInputDevice8W_RunControlPanel,
    .Initialize = IDirectInputDevice8W_Initialize,

    // IDirectInputDevice2W
    .CreateEffect = IDirectInputDevice8W_CreateEffect,
    .EnumEffects = IDirectInputDevice8W_EnumEffects,
    .GetEffectInfo = IDirectInputDevice8W_GetEffectInfo,
    .GetForceFeedbackState = IDirectInputDevice8W_GetForceFeedbackState,
    .SendForceFeedbackCommand = IDirectInputDevice8W_SendForceFeedbackCommand,
    .EnumCreatedEffectObjects = IDirectInputDevice8W_EnumCreatedEffectObjects,
    .Escape = IDirectInputDevice8W_Escape,
    .Poll = IDirectInputDevice8W_Poll,
    .SendDeviceData = IDirectInputDevice8W_SendDeviceData,

    // IDirectInputDevice7W
    .EnumEffectsInFile = IDirectInputDevice8W_EnumEffectsInFile,
    .WriteEffectToFile = IDirectInputDevice8W_WriteEffectToFile,

    // IDirectInputDevice8W
    .BuildActionMap = IDirectInputDevice8W_BuildActionMap,
    .SetActionMap = IDirectInputDevice8W_SetActionMap,
    .GetImageInfo = IDirectInputDevice8W_GetImageInfo,
};

static IDirectInputDevice8W stub_device_object = {
    .lpVtbl = &stub_device_vtbl,
};

IDirectInputDevice8W *di8_stub_device = &stub_device_object;
