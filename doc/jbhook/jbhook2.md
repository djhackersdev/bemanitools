# Game list

The following games are compatible with this version of jbhook:

- jubeat knit
- jubeat copious

The games must be bootstrapped using [launcher](../launcher.md).

# Data setup and running the game

Unpack the package containing jbhook2 into the revision folder of your choice. Most likely, you want
to target the latest revision you have to run the latest binary of the game with any bugfixes by
developers.

The DLLs in this package should be in the same location as the game DLLs and executable - i.e. all
these files should be in the same folder:

- `launcher.exe`
- `jubeat.dll`
- `eamio.dll`
- `jbhook2.dll`
- `libavs-win32.dll`
- etc

Run the `gamestart-02.bat` file as admin.

# Eamuse network setup

If you want to run the games online, you have to set a valid PCBID in the configuration file. You
also have to set the url of the eamuse server you want to connect to.

# Eamuse network setup

If you want to run the games online, you have to set a valid PCBID and EAMID (use the PCBID as the
EAMID) in the configuration file or as a command line argument. You also have to set the url of the
eamuse server you want to connect to.

Additional note regarding EAMID: This is provided as the identifier of the "eamuse license" to the
server. Depending on the implementation of the server, this can lead to authentication failure
resulting in a network error on boot or warning during gameplay.
