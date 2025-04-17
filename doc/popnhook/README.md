# popnhook

popnhook is a collection of hook libraries for pop'n music, providing ways to play the games on non BemaniPC, hardware and newer Windows versions.

## Supported version so far

15 ADVENTURE, 16 PARTY♪, 17 THE MOVIE, 18 せんごく列伝

We recommend using clean dumps to avoid any conflicts
with other cracks or binary patches.

## How to use Popnhook

You need to use an older version of popnhook (as of the time of writing, 4/17/25), as newer versions have broken compatiblity with older games.

[Verion 5.44 can be found here](https://github.com/djhackersdev/bemanitools/releases/download/5.44/bemanitools-5.44.zip).

Once downloaded, you have to open the game folder, and go into the `prog` folder. If there's multipe files here, you have to choose the one with the biggest number, which is typically the last one.

![image](https://github.com/user-attachments/assets/18b46242-a23d-4378-9d50-b283a8f2f94a) 

this happens when there's multiple versions of a game, you can use the hook for all versions, but you only really need to do the last one.

When you open the folder, you'll see a lot of different folders, you just need the `popn-15-to-28.zip` one. Simply extract the contents of the folder with the highest number, it'll look something like this

![image](https://github.com/user-attachments/assets/9f6bf715-0965-4a44-93c3-187e6c0e1b41)


Now that you have everything in the right spot, you choose which `gamestart-XX.bat` file you need for your game. You can delete the ones that aren't your game. To set your controls, you open the `config.exe` program in the same folder we've been working in. 

What if you don't want to open to the folder everytime you want to start the game? Simply right click the `gamestart-XX.bat`, hover over `Send to`, then select Desktop Shortcut. Now you can move the short cut anywhere you want, and when you open it, it'll boot up the game. Simple Stuff!

![image](https://github.com/user-attachments/assets/417bd195-fe89-43c0-bfa2-df9eb1eb5593)


## Trouble Shooting

You may run into a few errors, here's the more common problems, what they mean and how to fix them. If you want a full list of error codes the game may throw out, [You can see so here](https://github.com/djhackersdev/bemanitools/blob/master/doc/game-error-codes.md)

## 5-1502

![429665247-74a06c2d-c335-408a-91b1-c7ca6356d3f4](https://github.com/user-attachments/assets/b1f2f099-4068-4f17-8a38-af615effc10d)

This means game's security was missing something, it'll typically fail at security. It means that you didn't install the older version of bemanitools/popnhooker, and are using the latest version. Please install the older version, which can be found here.

[Verion 5.44 can be found here](https://github.com/djhackersdev/bemanitools/releases/download/5.44/bemanitools-5.44.zip).


## 5-1501

The game is trying to connect online, but is failing. This requires a few extra steps to fix. Go to your Firewall settings, and click `Allow an app through firewall`

![Captusre](https://github.com/user-attachments/assets/298f24ec-72e0-4e73-8c1a-6eac9a6a41d5)

Click `Change settings`, then `Allow another app...` and find select the `popnXX.exe` This is in the same folder you extracted your popnhooker. Then set the connection setting to Publc.

After that, you can boot up the game just fine. You can do a few things to remove it from the firewall if you care to do it (and doesn't show the e amuse prompt on boot up).

Once in the game, press the service menu option (you can set it to w/e you want it to be, in the `config.exe` program.

Once in the service menu, go to `Network Settings`

![430747057-3fe6be09-d74e-4c67-b011-17fb3876e811](https://github.com/user-attachments/assets/5cd396e9-d789-45fb-ab4e-2f034635ca57)

then e-amusement settings, and on this screen, turn OFF e-amusement

![430747092-4c85e4fa-a473-4aac-9ef5-7d9d214dbdd8](https://github.com/user-attachments/assets/73d96664-3564-4909-860b-418da90dded8)

Now, you can remove `popnXX.exe` from the firewall. Now you can boot up without any issue!



# Command line options

Add the argument *-h* when running inject with popnhook to print help/usage information with a list
of parameters you can apply to tweak various things.
