# Beatmania IIDX 10th Style - D01 IO boot code and security init

Date: 2023-04-03 Author: icex2

Documenting decompiled and reverse engineered code snippets from the D01 JAE `bm2dx.exe`. These
helped me figuring out the two different security boot modes the game supports.

In summary, it supports booting with the C02 IO with a C02 black dongle and a D01 IO board with a
D01 dongle. No other combination is valid because they didn't make sense back then. You either had
an upgraded old style/twinkle cabinet to C02 (GEC02) or you bought a new dedicated cabinet (GQD01).
With 10th style supporting the old C02 dongle, it appears that all owners of a C02 cabinet with IO
board and C02 dongle received a free software update/HDD. This might make sense considering the
short life span of C02 and the game being super buggy, especially in earlier/initial revisions.

The game expects the following "configurations" from bemanitools:

- Booting as upgraded C02 with free D01 upgrade
  - `sec.boot_version=GEC02   `
  - `sec.boot_seeds=0:0:1`
  - `sec.black_plug_mcode=GEC02JAA`
  - "D01 IO pin" on IO board not active
- Booting as dedicated
  - `sec.boot_version=GEC02   `
  - `sec.boot_seeds=0:1:1`
  - `sec.black_plug_mcode=GQD01JAA`
  - "D01 IO pin" on IO board ACTIVE

All the above was derived from reading and understanding the documented code excerpts below.

## io_boot - sub_402700

Called as the first step in the boot statemachine by the "boot" function `sub_40F9C0`.

```c
// sub_402700
int __stdcall io_boot(int a1)
{
  DWORD (__stdcall *timeGetTime)(); // esi
  int usb_boot_security_res; // ebp
  DWORD time_start; // ebx
  int io_boot_state; // edi
  DWORD now; // esi
  BOOL is_d01_boot_mode; // esi
  BOOL v7; // esi
  int result; // eax
  int v9; // [esp+10h] [ebp-2Ch]
  int firmware_version; // [esp+14h] [ebp-28h]
  int io_pad_read; // [esp+18h] [ebp-24h] BYREF
  char v12[32]; // [esp+1Ch] [ebp-20h] BYREF

  dword_4D0768 = 0;
  strcpy(byte_4D076C, &byte_4D0858);
  timeGetTime = ::timeGetTime;
  v9 = 0;
  usb_boot_security_res = io_pad_read;
  time_start = ::timeGetTime();
  io_boot_state = 0;
  while ( 2 )
  {
    switch ( io_boot_state )
    {
      case 0:                                   // "start io", firmware download and wait for reconnect
        if ( usbStart(0) )
        {
          if ( time_exceeded(time_start, 20000) )
            set_io_error_type(1, aErrorUsbioStar);// ERROR(USBIO START)
        }
        else
        {
          io_boot_state = 1;
        }
        goto reset_io_boot_state_label;
      case 1:                                   // check firmware
        firmware_version = usbFirmResult();
        if ( firmware_version == 96 )
        {
          if ( time_exceeded(time_start, 25000) )
            set_io_error_type(1, aErrorFm);     // ERROR(FM), FM = firmware
        }
        else
        {
          io_boot_state = 2;
        }
        goto reset_io_boot_state_label;
      case 2:
        if ( !firmware_version )
        {
          usbMute(1);
          time_start = timeGetTime();
          now = timeGetTime();
          io_input_is_d01_ezusb = 0;
          do
          {
            usbPadRead(&io_pad_read);
            if ( (io_pad_read & 0x10) != 0 )
              io_input_is_d01_ezusb = 1;
          }
          while ( !time_exceeded(now, 2000) );
          io_boot_state = 3;
reset_io_boot_state_label:
          timeGetTime = ::timeGetTime;
          if ( io_boot_state != v9 )
          {
            v9 = io_boot_state;
            time_start = ::timeGetTime();
          }
          continue;                             // retry
        }
        if ( firmware_version > 128 )
        {
          if ( firmware_version == 254 )
          {
            set_io_error_type(1, aErrorFmTrnsOut);// ERROR(FM TRNS-OUT)
            return 1;
          }
          if ( firmware_version == 255 )
          {
            set_io_error_type(1, aErrorFmTimeOut);// ERROR(FM TIME-OUT)
            return 1;
          }
        }
        else
        {
          switch ( firmware_version )
          {
            case 128:
              set_io_error_type(1, aErrorFmReadErr);// ERROR(FM READ-ERR)
              return 1;
            case -113:
              set_io_error_type(1, aErrorFmDlErr);// ERROR(FM DL-ERR)
              return 1;
            case -33:
              set_io_error_type(1, aErrorFmCmprReq);// ERROR(FM CMPR-REQ)
              return 1;
          }
        }
UNKNOWN_ERROR_LABEL:
        set_io_error_type(1, aErrorUnknown);    // ERROR(UNKNOWN)
        return 1;
      case 3:
        if ( io_init_security() )
        {
          set_io_error_type(1, aErrorSqInit);   // ERROR(SQ-INIT)
          return 1;
        }
        io_boot_state = 4;
        goto reset_io_boot_state_label;
      case 4:
        is_d01_boot_mode = io_input_is_d01_ezusb != 0;
        if ( !strncmp(black_dongle_mcode, MCODE_GED01, 5u) )
          is_d01_boot_mode = 1;
        usb_boot_security_res = usbBootSecurity(aGec02, 0, is_d01_boot_mode, 1);
        if ( usb_boot_security_res == 96 )
        {
          if ( time_exceeded(time_start, 20000) )
            set_io_error_type(1, aErrorBtSqInit);// ERROR(BT-SQ-INIT)'
        }
        else
        {
          io_boot_state = 5;
        }
        goto reset_io_boot_state_label;
      case 5:
        if ( usb_boot_security_res )
        {
          switch ( usb_boot_security_res )
          {
            case 0xFFFFFE9F:
              set_io_error_type(1, aErrorSecurityE);// ERROR(SECURITY EEP)
              result = 1;
              break;
            case 0xFFFFFECF:
            case 0xFFFFFEDF:
              set_io_error_type(1, aErrorSecurityI);// ERROR(SECURITY ID or EEP)
              result = 1;
              break;
            case 0xFFFFFEEF:
              set_io_error_type(1, aErrorSecurityN);// 'ERROR(SECURITY No Match)'
              result = 1;
              break;
            case 0xFFFFFEFF:
              io_boot_state = 6;
              goto reset_io_boot_state_label;
            default:
              goto UNKNOWN_ERROR_LABEL;
          }
        }
        else
        {
          Sleep(0x3E8u);
          result = 0;
        }
        return result;
      case 6:                                   // security conversion/upgrading security dongles
        v7 = io_input_is_d01_ezusb != 0;
        if ( !usbGetSecurityKey(v12) )
        {
          if ( usbSetupSecurityComplete(v12, 0, v7, 1) )
            set_io_error_type(1, aErrorSecurityC);// ERROR(SECURITY CONVERSION FAILED)
          return 1;
        }
        set_io_error_type(1, aErrorSecurityC);  // ERROR(SECURITY CONVERSION FAILED)
        goto reset_io_boot_state_label;
      default:
        goto UNKNOWN_ERROR_LABEL;
    }
  }
}
```

## io_init_security - sub_402BB0

Sub-function of the [io_boot](#io_boot---sub_402700) function.

```c
// sub_402BB0
int io_init_security()
{
  int v0; // esi
  int v2; // esi
  int mcode_mismatch_cnt; // esi
  char white_dongle_data_maybe[10]; // [esp+Ch] [ebp-20h] BYREF

  usbSecurityInit();
  v0 = 0;
  while ( usbSecurityInitDone() )
  {
    if ( v0 > 1200 )
    {
      set_io_error_type(-1, aNgSi);
      return -1;
    }
    ++v0;
    Sleep(16u);
  }
  pcbid = 0;
  dword_4D0754 = 0;
  word_4D0758 = 0;
  while ( usbGetPCBID(&pcbid) )
    Sleep(1u);
  *(_DWORD *)black_dongle_mcode = 0;
  v2 = 0;
  *(_DWORD *)&black_dongle_mcode[4] = 0;
  while ( usbGetSecurity(black_dongle_mcode) )  // black_dongle_mcode = GQD01JAA
  {
    if ( v2 > 1200 )
    {
      set_io_error_type(-1, aNgPd);
      return -1;
    }
    ++v2;
    Sleep(16u);
  }
  set_cabinet_type(black_dongle_mcode[6]);
  mcode_mismatch_cnt = black_dongle_mcode[0] != 'G';
  if ( io_input_is_d01_ezusb || !strncmp(black_dongle_mcode, MCODE_GED01, 5u) )// Path for using the D01 dongle with D01 IO board
  {
    if ( (unsigned __int8)black_dongle_mcode[2] != (char)MCODE_D01 )
      ++mcode_mismatch_cnt;
    if ( (unsigned __int8)black_dongle_mcode[3] != SBYTE1(MCODE_D01) )
      ++mcode_mismatch_cnt;
    if ( (unsigned __int8)black_dongle_mcode[4] != SBYTE2(MCODE_D01) )
      ++mcode_mismatch_cnt;
  }
  else                                          // Path for using the C02 dongle with C02 IO board
  {
    if ( (unsigned __int8)black_dongle_mcode[2] != MCODE_C02[0] )
      ++mcode_mismatch_cnt;
    if ( (unsigned __int8)black_dongle_mcode[3] != MCODE_C02[1] )
      ++mcode_mismatch_cnt;
    if ( (unsigned __int8)black_dongle_mcode[4] != MCODE_C02[2] )
      ++mcode_mismatch_cnt;
    if ( !usbGetSecurityKey(white_dongle_data_maybe) )
    {
      black_dongle_mcode[6] = white_dongle_data_maybe[6];
      set_cabinet_type(white_dongle_data_maybe[6]);
    }
  }
  if ( black_dongle_mcode[5] != 'J' )
    ++mcode_mismatch_cnt;
  if ( black_dongle_mcode[6] != 'A' && black_dongle_mcode[6] != 'B' && black_dongle_mcode[6] != 'C' )
    ++mcode_mismatch_cnt;
  if ( !mcode_mismatch_cnt )
    return 0;
  set_io_error_type(-1, aNgSecurity);
  return -1;
}
```
