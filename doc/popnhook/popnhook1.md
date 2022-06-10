# Game list

The following games are compatible with this version of popnhook:
* pop'n music 15 ADVENTURE
* pop'n music 16 PARTY♪
* pop'n music 17 THE MOVIE
* pop'n music 18 せんごく列伝

The games must be bootstrapped using [inject](../inject.md).

# Data setup and running the game

Unpack the package containing popnhook1 into the revision folder of your choice.
Most likely, you want to target the latest revision you have to run the latest
binary of the game with any bugfixes by developers.

The DLLs in this package should be in the same location as the game DLLs and
executable - i.e. all these files should be in the same folder:
- `inject.exe`
- `popn15.exe` or `popn16.exe` or `popn17.exe` or `popn18.exe`
- `popnhook-15.conf` or `popnhook-16.conf` or `popnhook-17.conf` or `popnhook-18.conf`
- `eamio.dll`
- `popnio.dll`
- `libavs-win32.dll`
- etc

Run the `gamestart-15.bat` or `gamestart-16.bat` or `gamestart-17.bat` or `gamestart-18.bat` file as admin.

# Eamuse network setup

If you want to run the games online, you have to set a valid PCBID in the
configuration file. You also have to set the url of the eamuse server you want
to connect to.

# Eamuse network setup

If you want to run the games online, you have to set a valid PCBID and EAMID
(use the PCBID as the EAMID) in the configuration file or as a command line
argument. You also have to set the url of the eamuse server you want to
connect to.

Additional note regarding EAMID: This is provided as the identifier of the
"eamuse license" to the server. Depending on the implementation of the server,
this can lead to authentication failure resulting in a network error on boot or
warning during gameplay.
