- cut a release before dropping these changes as the impact is huge

-> todo: move all of the below to a bt6 branch?

- add development build container

- rework logging system, issues with non-async logging and lagging the game (stdout and file must be written async),
  implement features such as log ration etc. as outlined by the bootstrap logging config
- have some sort of migration guide to switch to the new launcher. part of that should be proper
  documentation about how to use the launcher the configuration xml file and what you can do
  with it with various use-case examples
- align layout of distribution files, different for games such as iidx, jubeat and sdvx

// TODO redact stuff like PCBID

config format with stacking and overriding doesn't work here:
    <fs>
        <nr_filesys __type="u16">16</nr_filesys>
        <nr_mountpoint __type="u16">1024</nr_mountpoint>
        <nr_mounttable __type="u16">32</nr_mounttable>
        <nr_filedesc __type="u16">1024</nr_filedesc>
        <link_limit __type="u16">8</link_limit>
        <root>
            <device __type="str">.</device>
        </root>
        <mounttable_selector __type="str">boot</mounttable_selector>
        <mounttable>
            <vfs name="boot" fstype="fs" src="D:/LDJ/contents/dev/raw" dst="/dev/raw" opt="vf=1,posix=1"/>
            <vfs name="boot" fstype="fs" src="D:/LDJ/contents/dev/nvram" dst="/dev/nvram2" opt="vf=0,posix=1"/>
            <vfs name="boot" fstype="nvram2" src="/dev/nvram2" dst="/dev/nvram" opt="nr_mirror=4"/>
        </mounttable>
    </fs>

vfs nodes are being replaced as they are not unique by node name -> add tag __replace="true" or something similar to
<config kind="inline" replace="true">