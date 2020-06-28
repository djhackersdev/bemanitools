# Using SMX pads with BemaniTools (DDR Ace and higher)
If you are looking to use Stepmaniax pads with DDR, these are supported **natively** for Gen 1 through Gen 5.(And theoretically beyond!) Steps and info below:

## Step-by-Step:
1. Ensure that you have the latest SMX.dll copied to your contents folder. The easiest way to get this is to download and install the latest SMX Config program on [Stepmaniax.com](https://data.stepmaniax.com/docs/SMXConfigInstaller-2020-04-03-01.exe). SMX.dll's default install location is `C:\Program Files (x86)\SMXConfig`
2. After bemanitools has been copied to your contents folder, rename the regular ddrio.dll to something like ddrio-orig.dll.
3. Rename ddrio-smx.dll to ddrio.dll.
4. Use the config tool to set up the usual stuff.(cards, network, etc.) Be sure to map menu, test, service, and start buttons. Note: You **cannot** map SMX panels as joystick inputs.
5. Start the game. If everything works properly, the game will boot and your panels will light up when stepped on during gameplay.

## Important Things:
* Pad panel inputs (UDLR on P1 and P2) are mapped automatically. If you've purchased a single pad from Stepmaniax, this jumper is installed and will set your pad to P2 by default. If you want to change to P1, you'll need to open up your pads and remove this jumper in the [MCU box](https://data.stepmaniax.com/docs/Stage%20-%20Gen%205%20Manual%20Rev1.pdf).
* Panel colors set in the SMX config tool are not used. Colors are set internally by Bemanitools.
* If you get a warning about setlights2 when starting the game, your SMX.dll is likely out of date. Make sure you've downloaded the latest version of the Stepmaniax config tool from stepmaniax.com and have copied the SMX.dll to your contents folder.