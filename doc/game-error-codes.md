# Bemani Game Error Codes

This document collects the various error codes that are know across the many games including some
information what can be done.

This information is not part of bemanitools but useful to many users using it to run the games. We
saw users encountering these errors not knowing what they mean. Therefore, we include these errors
in the repository to provide an accessible source of information regarding them.

Important: The error codes are not consistent throughout the many years of game series and versions.

It is adviced to read the error information carefully (use Google Translate App with your phone's
camera) and check the following sections. It would be great to get feedback if you see information
missing or specific games and versions not matching to increase the value of the list.

## 5-0000 series errors

### 5-0000-0000

#### Description

A generic critical error.

#### Known occurances

N/A

#### How to fix

* Make sure your data is not set to read only, especially the dev folder.
* Disable all network adapters other than the one used to connect to the Internet.
* Check for hidden network adapters in Device Manager.

## 5-1500 series errors

### 5-1500-0000

#### Description

IO ERROR

#### Known occurances

* Pop'n Music 15

#### How to fix

No information available.

### 5-1500-0002

#### Description

SOUND DATA CREATE ERROR

#### Known occurances

N/A

#### How to fix

* Check the integrity of the data you have, i.e. some files might be corrupted
* Check your physical drive. Is it failing?

## 5-1501 series errors

### 5-1501-0000

#### Description

IO BOARD ERROR

#### Known occurances

N/A

#### How to fix

* Recopy Bemanitools.
* For Reflec Beat, make sure you are not missing device.dll. (This seems to apply to some older
version of bemanitools)

## 5-1502 series errors

### 5-1502-0000

#### Description

BACKUP CHECK ERROR

#### Known occurances

* Pop'n'Music 21 Sunny Park

#### How to fix

* Give access to gamestart.bat and launcher.exe from Windows Firewall if enabled.
* Make sure no antivirus software firewall is blocking access to those files.
* Start gamestart.bat as Administrator.

### 5-1502-0000

#### Description

HDD READ ERROR

#### Known occurances

N/A

#### How to fix

* Make sure `/dev/` isn't read-only.

### 5-1502-0002

#### Description

HDD READ ERROR

#### Known occurances

* GFDM V4

#### How to fix

Unknown

## 5-1503 series errors

### 5-1503-0000

#### Description

USB I/O ERROR 

##### FPGA WRITE ERROR

Writing the FPGA firmware fails. Can have various causes depending on your setup.

##### FW TRNS-OUT

This typically happens if it takes too long to upload the base firmware to the USB IO. A potential
root-cause can be bad timing since the game uses a bad combination of software sleeps and fixed
timeouts making this code prone to fail.

#### Known occurances

* IIDX

#### How to fix

#### FPGA WRITE ERROR

* Restart the game
* Re-copy bemanitools

#### FW TRNS-OUT

* Restart the game
* Re-copy bemanitools
* Running the process on high priority and with admin rights
* If you attached a debugger, that might have messed with the timing and caused this. Try a
different debugger or attach it later in the boot process

### 5-1503-0001

#### Description

DATA ERROR

"Could not read the file normally. There is a likelihood the file is damaged."

This error occurs when a file is either missing or corrupt.

#### Known occurances

N/A

#### How to fix

* Check the integrity of the data you have, i.e. some files might be corrupted
* If the data came in an archive format, try extracting it again with another application.
* Make sure your game is up to date with the current data available.
* Check your physical drive. Is it failing?

### 5-1503-0004

#### Description

USBIO ERROR (NO ANSWER...)

This error occurs when the game does not receive a heartbeat from the IO board. On legitimate
hardware, this error is usually caused by the IO board resetting during play. The issue is rarely
fixed by simply restarting the game and will require a full power cycle.

#### Known occurances

N/A

#### How do I fix it?

Real hardware:

* Check the USB cable between the PC and the IO board is okay.
* Make sure the IO board is getting enough of a clean supply of power.

Bemanitools:

* Don't attempt to put your computer to sleep while playing rhythm games.
* Change launcher.exe process priority to High when starting the game.

### 5-1503-0006

#### Description

CARD DEVICE ERROR

#### Known occurances

N/A

#### How do I fix it?

* Recopy Bemanitools.

### 5-1503-0007

#### Description

USBIO ERROR (FM DL-ERR)

#### Known occurances

N/A

#### How do I fix it?

* Run the game using gamestart.bat, don't just drag and drop the game dll onto launcher.exe

### 5-1503-0008

#### Description

FPGA Write error

#### Known occurances

* IIDX Tricoro

Seen in Tricoro Omnimix RC1.

#### How do I fix it?

Unknown

### 5-1503-9000

### Description

PCB ERROR (PCB is unusual.)

#### Known occurances

N/A

#### How do I fix it?

* Make sure your AC(real)IO board is on the correct COM port.

## 5-1504 series errors

### 5-1504-0000

#### Description

IC CARD READER ERROR

Something is wrong with your e-amusement card readers (usually a timeout due to them not existing).

#### Known occurances

N/A

#### How do I fix it?

Real hardware:

* Make sure your readers are connected and powered on.
* Check your serial cables are good.

### 5-1504-0003

### Description

N/A

#### Known occurances

N/A

#### How do I fix it?

N/A

### 5-1504-0004

### Description

SDLOAD TIMEOUT or SOUND LOAD TIMEOUT

#### Known occurances

N/A

#### How do I fix it?

N/A

## 5-1505 series errors

### 5-1505-0000

#### Description

HDD Data Error(BPM Notes)

For Jubeat Ripples, getting this error the first time you boot is normal. After getting this error,
the game may take 1-3 minutes longer to boot as it regenerates backup data.

#### Known occurances

* Jubeat Ripples

#### How do I fix it?

* Check the integrity of the data you have, i.e. some files might be corrupted
* If the data came in an archive format, try extracting it again with another application.
* Check your physical drive. Is it failing?

### 5-1505-0001

#### Description

HDD Data Error(Music Data)

#### Known occurances

N/A

#### How do I fix it?

* Check the integrity of the data you have, i.e. some files might be corrupted
* If the data came in an archive format, try extracting it again with another application.
* Check your physical drive. Is it failing?

### 5-1505-0002

#### Description

Coin Error

This error occurs when the coin line (the input triggered when a coin is inserted) is held closed
for too long.

#### Known occurances

N/A

#### How do I fix it?

##### Real hardware

* There may be a coin stuck, check.
* The coin mechanism may be stuck, check.
* Don't hold down the coin line so long.

### 5-1505-0006

#### Description

HDD DATA ERROR(MOUNT IMAGEFS)

#### Known occurances

N/A

#### How do I fix it?

* Check the integrity of the data you have, i.e. some files might be corrupted
* If the data came in an archive format, try extracting it again with another application.
* Check your physical drive. Is it failing?

### 5-1505-0025

#### Description

N/A

#### Known occurances

* Jubeat Saucer Fullfill

#### How do I fix it?

* Disable excess network adapters

## 5-1506 series errors

### 5-1506-0000

#### Description

##### ACIO ERROR (ERR_INIT_LINE)

##### BACKUP DATA ERROR

The backup data is corrupted. Press the test button to re-initialize it.

#### Known occurances

* IIDX 15 (BACKUP DATA ERROR)

#### How do I fix it?

##### ACIO ERROR

If you're using ACrealIO, cut the reset trace on your Arduino.

##### BACKUP DATA ERROR

Follow the instructions of the error message. You will have to re-set certain settings like clock
settings in the operator menu to complete initialization.

### 5-1506-0001

#### Description

SHOP NAME ERROR

This error may happen before the monitor check on IIDX.

#### Known occurances

* IIDX (version?)

#### How do I fix it?

Press the service button and set the shop name in "NETWORK OPTIONS".

### 5-1506-1001

#### Description

CLOCK ERROR

#### Known occurances

N/A

#### How do I fix it?

* "Set" and save the clock date and time in the test menu
* Recopy Bemanitools

## 5-1507 series errors

### 5-1507-0000

#### Description

SECURITY ERROR

#### Known occurances

N/A

#### How do I fix it?

Make sure your ea3-config.xml file contains valid values on the following lines:

```xml
<dest __type="str">...</dest>
```

Should almost always be `J`. Certain games will support `K` (Korea) or `A` (Asia) but there is
rarely a reason to use it. DDR 2014 and forward will support `U`. DanEvo will support (I believe)
`C`.

```xml
<spec __type="str">...</spec>
```

Should almost always be `A`. DDR will support `B` for non-HD monitors.

```xml
<rev __type="str">...</rev>
```

Should always be `A`.

## 5-1508 series errors

### 5-1508-0000

#### Description

LAMP CHECK ERROR

#### Known occurances

* Pop'n Music 20 Fantasia

#### How do I fix it?

N/A

## 5-1509 series errors

### 5-1509-0000

#### Description

REBOOT THE MACHINE

The original message in Japanese: 電源を再投入して下さい

#### Known occurances

* Pop'n Music 22 Lapistoria

#### How do I fix it?

Restart entire machine if on cabinet, or close the game (ALT+F4/ALT+TAB and then close through
taskbar) and re-run the launcher/gamestart file

## 5-2000 series errors (Network errors)

### 5-2000-0000

#### Description

Router error.

Something's not okay with the connection between the game and your router. This can either be a
(W)LAN error, or something within your computer settings (firewall, network adapter, etc.)

#### Known occurances

N/A

#### How do I fix it?

* Restart your game.
* Check if your computer is connected to the Internet at all.
* Make sure the services URL is correct.
* Disable all network adapters other than the one used to connect to the Internet. Alternatively,
setting that adapter to have the highest affinity also works.
* If none of the above work, reinstall a newer or older version of your network drivers.

### 5-2002-0000

#### Description

speculation: general malformed ea3-config.xml file or server communication error

#### Known occurances

N/A

#### How do I fix it?

* Make sure you're editing the right file for network configuration settings.

### 5-2002-0916

#### Description

Cannot communicate with the server.

#### Known occurances

N/A

#### How do I fix it?

N/A

### 5-2002-1013

#### Description

NETWORK ERROR

Cannot communicate with server.

Ensure that your internet connection is working and that your router is turned on.

Ensure that your machine is connected to your router.

Press the service button to start the game.

#### Known occurances

* IIDX, probably all known versions

#### How do I fix it?

The original message is already telling you what is going on:

* Check if your machine/pc has an internet connection
* Check your firewall settings
* Check that the server address is configured and correct in `iidxhook.conf` (old games) or
`ea3-config.xml`
* You can disable the network in the operator menu to just boot the game first. Note that this is
not possible on any versions starting IIDX 20 (tricoro).

### 5-2002-2301

#### Description

Cannot communicate with the server.

#### Known occurances

N/A

#### How do I fix it?

Your ea3 services URL is probably incorrect.

### 5-2000-2402

#### Description

No PCBID. You have no PCBID.

#### Known occurances

N/A

#### How do I fix it?

Add a valid PCBID to `ea3-config.xml`.

### 5-2002-0910

#### Description

Server Error. The game cannot communicate with the server.

#### Known occurances

N/A

#### How do I fix it?

Make sure the services URL is correct.

### 5-2002-2403

#### Description

Unauthorized. The server is rejecting your PCBID.

#### Known occurances

N/A

#### How do I fix it?

* Check that there is a valid PCBID in ea3-config.xml.

### 5-2002-2400

#### Description

Bad Request. Syntax was entered improperly and the server does not understand it.

#### Known occurances

N/A

#### How do I fix it?

Check your services URL and make sure it is entered correctly (e.g. "http:" instead of "https:")

### 5-2002-2404

#### Description

Not found. The server can't find the URL that the game has requested, this is usually because the
game is attempting to connect to an incorrect endpoint.

#### Known occurances

N/A

#### How do I fix it?

* Check your services URL in `ea3-config.xml`. Make sure to remove any slash at the end of the URL
and check for https/http.
* If you are running Spada, ensure to use a service URL that supports that older game (check with
your private server provider)
* If you are running Tune Street, ensure to use a service URL that supports that older game (check
with your private server provider)

### 5-2003-2404

#### Description

Malformed `<network>` section.  The `<network>` section in your `ea3-config.xml` file is malformed
in some way.

#### Known occurances

N/A

#### How do I fix it?

Ensure your `<network>` section follows the following format:

```xml
<network>
	<timeout __type="u32">10000</timeout>
	<sz_xrpc_buf __type="u32">102400</sz_xrpc_buf>
	<ssl __type="bool">0</ssl>
	<services></services>
	<url_slash __type="bool">1</url_slash>
</network>
```

Note: Delete the line `<url_slash __type="bool">1</url_slash>` if you're connecting to Arcana

### 5-2002-2500

#### Description

??? (Logs say http status 500)

#### Known occurances

N/A

#### How do I fix it?

If you are playing Lincle, use the `/core/services` URL from Tricoro, not xrpc.

### 5-2003-2502

#### Description

Server Error. The game cannot communicate with the server.

#### Known occurances

N/A

#### How do I fix it?

Check if your `hosts` file has any DNS redirects to other private servers that might interfere
with this

### 5-2016-0000

#### Description

"apsmanager" is a special eAM Participation system for Korean machines(XXX:K). This system
is maybe not supported by private servers.

#### Known occurances

N/A

#### How do I fix it?

Make sure your `ea3-config.xml` has this line set to 0:

```xml
<apsmanager __type="u8">0</apsmanager>
```
