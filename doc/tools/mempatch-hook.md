# A simple memory patching hook

This is a hook which can be passed along with other hooks to be injected into the target application
of your choice (e.g. when using inject or launcher). It allows you to patch raw memory contents of
either the target application or any libraries loaded with it. No static hex-edits anymore. Instead,
create a simple script file and also document your patches for others which allows them to easily
disable/enable them.

# Setup

Copy the *mempatch-hook.dll* to the target application of your choice and add it to the list of
libraries to inject:

Example when using *inject.exe*:

```
inject iidxhook3.dll mempatch-hook.dll bm2dx.exe --config iidxhook-16.conf --mempatch myPatch.mph %*
```

When using *launcher.exe*:

```
launcher -K iidxhook4.dll -K mempatch-hook.dll bm2dx.dll --config iidxhook.conf --mempatch myPatch.mph %*
```

To load a patch script, add the *--mempatch <path to patch script>* argument (as shown above in the
example). You can specify this more than once which allows you to apply multiple scripts in order,
e.g. *--mempatch myPatch1.mph --mempatch myPatch1.mph*.

# Patch script format

A patch script is a simple list of items seperated by a newline character (i.e. one item = one
line). Example script file:

```
# This is a comment
# Use comments to document your patches and make the script useful for others

# Empty lines are allowed as well and skipped by the patcher

# All numbers specified are hex format only.

# First entry which gets processed by the patcher. One entry specifies a single
# patch to apply starting a the specified address
# The first parameter (bm2dx.exe) is the base address. Specify the exe name
# of the application for relative addreses to patch inside the exe. You can
# also specify dlls loaded by the target application (e.g. libacio.dll)
#
# The second parameter (137C4C) is the offset. The target address for this patch
# is bm2dx.exe + 137C4C (bm2dx.exe commonly resolves to 400000) -> 5137C4C
#
# The third parameter is the data to patch at the target location. This byte hex 
# string can have an arbitrary even length.
#
# The fourth parameter is optional and allows you to specify the expected data
# at the target loation before patching. This gives you the chance to add some
# sort of checksum'ing for the patches if you want.
bm2dx.exe 137C4C 2121212121 4540

# The first parameter can also be - which resolves to a base memory address of
# 0 for the loaded application.
# So the target address here is 0 + 537C4C = 537C4C
#
# The fourth parameter isn't used here (optional)
- 537C4C 2222212122 

# You can also set the third parameter to '-' which means no data and disables
# the patching. This allows you to use the fourth parameter and execute a 
# memory check, only. This can be used for signiture checking of the target
# application
bm2dx.exe 137C4C - 4540
```
