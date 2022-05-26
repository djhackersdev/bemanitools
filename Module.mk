# Example AVS version: 2.13.4 (use std terminology: major.minor.patch)
#
# AVS major version has been 2 since forever
# AVS patch versions appear to maintain API and ABI compatibility.
# Or, they did until 2.16.7 reared its fucking head.
#
# So "AVS version" NOW equals minor version * 100 + patch version.
# 2.16.7 is encoded as 1607 under this scheme.
#
# Games with no AVS version (like IIDX 9th to Happy Sky) are treated as
# version 0.
#
# List of known versions:
#
#  None (0):        beatmania IIDX 9th Style
#                   beatmania IIDX 10th Style
#                   beatmania IIDX 11 RED
#                   beatmania IIDX 12 Happy Sky
#
#  1:   Patch 4:    beatmania IIDX 13 DistorteD
#
#  4:   Patch 2:    beatmania IIDX 14 GOLD
#
#  6:   Patch 5:    beatmania IIDX 15 DJ Troopers
#
#  8:   Patch 3:    beatmania IIDX 16 Empress
#                   jubeat
#
# 10:   Patch 2:    jubeat Knit
#       Patch 4:    DanceDanceRevolution X2
#
# 11:   Patch 1:    beatmania IIDX 18 Resort Anthem
#                   pop'n music 19 Tune Street
#
# 12:   Patch 1:    LovePlus Arcade (not a Bemani, w/e)
#
# 13:   Patch 4:    beatmania IIDX 19 Lincle
#       Patch 6:    DanceDanceRevolution X3
#                   pop'n music 20 Fantasia
#
# 14:	Patch 3:    Guitar Freaks & Drum Mania XG3
#
# 15:   Patch 8:    DanceDanceRevolution (2013)
#                   beatmania IIDX 20 tricoro
#                   pop'n music 21 Sunny Park
#                   SOUND VOLTEX (1-4)
#					jubeat prop
#
# 16:   Patch 1:    beatmania IIDX 21 SPADA
#                   beatmania IIDX 22 PENDUAL
#					beatmania IIDX 23 copula
#                   beatmania IIDX 24 SINOBUZ
#     "Patch" 3:    Silent Scope Bone Eater (compatible with p7)
#	  "Patch" 7:    Steel Chronicle VicTroopers (not a Bemani, w/e)
#
# 17:   Patch 0:    beatmania IIDX 25 CANNON BALLERS
#                   beatmania IIDX 26 Rootage
#                   beatmania IIDX 27 HEROIC VERSE
#
#       Patch 3:    SOUND VOLTEX VIVID WAVE
#
#

cflags          += \
	-DWIN32_LEAN_AND_MEAN \
	-DWINVER=0x0601 \
	-D_WIN32_WINNT=0x0601 \
	-DCOBJMACROS \
	-Wno-attributes \

# List only the AVS versions that are meaningfully distinct here.
# Each AVS-dependent project should consume the earliest AVS import definition
# that is still ABI-compatible with the real build its target links against.

avsvers_32      := 1700 1603 1601 1508 1403 1304 1101 1002 803 0
avsvers_64      := 1700 1603 1601 1509 1508

imps            += avs avs-ea3

include src/main/aciodrv/Module.mk
include src/main/aciodrv-proc/Module.mk
include src/main/acioemu/Module.mk
include src/main/aciomgr/Module.mk
include src/main/aciotest/Module.mk
include src/main/asio/Module.mk
include src/main/bio2drv/Module.mk
include src/main/bio2emu-iidx/Module.mk
include src/main/bio2emu/Module.mk
include src/main/bsthook/Module.mk
include src/main/bstio/Module.mk
include src/main/camhook/Module.mk
include src/main/cconfig/Module.mk
include src/main/config/Module.mk
include src/main/d3d9exhook/Module.mk
include src/main/ddrhook-util/Module.mk
include src/main/ddrhook1/Module.mk
include src/main/ddrhook2/Module.mk
include src/main/ddrio-mm/Module.mk
include src/main/ddrio-smx/Module.mk
include src/main/ddrio/Module.mk
include src/main/dinput/Module.mk
include src/main/eamio-icca/Module.mk
include src/main/eamio/Module.mk
include src/main/eamiotest/Module.mk
include src/main/ezusb-emu/Module.mk
include src/main/ezusb-iidx-emu/Module.mk
include src/main/ezusb-iidx-fpga-flash/Module.mk
include src/main/ezusb-iidx-sram-flash/Module.mk
include src/main/ezusb-iidx/Module.mk
include src/main/ezusb-tool/Module.mk
include src/main/ezusb/Module.mk
include src/main/ezusb2-dbg-hook/Module.mk
include src/main/ezusb2-emu/Module.mk
include src/main/ezusb2-iidx-emu/Module.mk
include src/main/ezusb2-iidx/Module.mk
include src/main/ezusb2-tool/Module.mk
include src/main/ezusb2/Module.mk
include src/main/geninput/Module.mk
include src/main/hook/Module.mk
include src/main/hooklib/Module.mk
include src/main/iidx-bio2-exit-hook/Module.mk
include src/main/iidx-ezusb-exit-hook/Module.mk
include src/main/iidx-ezusb2-exit-hook/Module.mk
include src/main/iidx-irbeat-patch/Module.mk
include src/main/iidxhook-util/Module.mk
include src/main/iidxhook1/Module.mk
include src/main/iidxhook2/Module.mk
include src/main/iidxhook3/Module.mk
include src/main/iidxhook4/Module.mk
include src/main/iidxhook5/Module.mk
include src/main/iidxhook6/Module.mk
include src/main/iidxhook7/Module.mk
include src/main/iidxhook8/Module.mk
include src/main/iidxhook9/Module.mk
include src/main/iidxio-bio2/Module.mk
include src/main/iidxio-ezusb/Module.mk
include src/main/iidxio-ezusb2/Module.mk
include src/main/iidxio/Module.mk
include src/main/iidxiotest/Module.mk
include src/main/inject/Module.mk
include src/main/jbio-magicbox/Module.mk
include src/main/jbio-p4io/Module.mk
include src/main/jbio/Module.mk
include src/main/jbiotest/Module.mk
include src/main/jbhook-util/Module.mk
include src/main/jbhook-util-p3io/Module.mk
include src/main/jbhook1/Module.mk
include src/main/jbhook2/Module.mk
include src/main/jbhook3/Module.mk
include src/main/launcher/Module.mk
include src/main/mempatch-hook/Module.mk
include src/main/mm/Module.mk
include src/main/p3io/Module.mk
include src/main/p3ioemu/Module.mk
include src/main/p4iodrv/Module.mk
include src/main/p4ioemu/Module.mk
include src/main/pcbidgen/Module.mk
include src/main/sdvxhook/Module.mk
include src/main/sdvxhook2-cn/Module.mk
include src/main/sdvxhook2/Module.mk
include src/main/sdvxio-bio2/Module.mk
include src/main/sdvxio-kfca/Module.mk
include src/main/sdvxio/Module.mk
include src/main/security/Module.mk
include src/main/unicorntail/Module.mk
include src/main/util/Module.mk
include src/main/vefxio/Module.mk
include src/main/vigem-iidxio/Module.mk
include src/main/vigem-sdvxio/Module.mk
include src/main/vigemstub/Module.mk

include src/test/cconfig/Module.mk
include src/test/d3d9hook/Module.mk
include src/test/iidxhook-util/Module.mk
include src/test/iidxhook8/Module.mk
include src/test/security/Module.mk
include src/test/test/Module.mk
include src/test/util/Module.mk

#
# Distribution build rules
#

zipdir          := $(BUILDDIR)/zip

$(zipdir)/:
	$(V)mkdir -p $@

$(zipdir)/tools.zip: \
		build/bin/indep-32/aciotest.exe \
		build/bin/indep-32/eamiotest.exe \
		build/bin/indep-32/ezusb-iidx-fpga-flash.exe \
		build/bin/indep-32/ezusb-iidx-sram-flash.exe \
		build/bin/indep-32/iidxiotest.exe \
		build/bin/indep-32/iidx-bio2-exit-hook.dll \
		build/bin/indep-32/iidx-ezusb-exit-hook.dll \
		build/bin/indep-32/iidx-ezusb2-exit-hook.dll \
		build/bin/indep-32/jbiotest.exe \
		build/bin/indep-32/pcbidgen.exe \
		dist/iidx/ezusb-boot.bat \
		dist/iidx/ezusb2-boot.bat \
		build/bin/indep-32/mempatch-hook.dll \
		build/bin/indep-32/ezusb2-dbg-hook.dll \
		build/bin/indep-32/ezusb2-tool.exe \
		build/bin/indep-32/ezusb-tool.exe \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/tools-x64.zip: \
		build/bin/indep-64/aciotest.exe \
		build/bin/indep-64/eamiotest.exe \
		build/bin/indep-64/iidxiotest.exe \
		build/bin/indep-64/iidx-bio2-exit-hook.dll \
		build/bin/indep-64/iidx-ezusb-exit-hook.dll \
		build/bin/indep-64/iidx-ezusb2-exit-hook.dll \
		build/bin/indep-64/jbiotest.exe \
		build/bin/indep-64/mempatch-hook.dll \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/iidx-09-to-12.zip: \
		build/bin/indep-32/iidxhook1.dll \
		build/bin/indep-32/config.exe \
		build/bin/indep-32/eamio.dll \
		build/bin/indep-32/geninput.dll \
		build/bin/indep-32/iidxio.dll \
		build/bin/indep-32/vefxio.dll \
		build/bin/indep-32/inject.exe \
		dist/iidx/config.bat \
		dist/iidx/gamestart-09.bat \
		dist/iidx/gamestart-10.bat \
		dist/iidx/gamestart-11.bat \
		dist/iidx/gamestart-12.bat \
		dist/iidx/iidxhook-09.conf \
		dist/iidx/iidxhook-10.conf \
		dist/iidx/iidxhook-11.conf \
		dist/iidx/iidxhook-12.conf \
		dist/iidx/vefx.txt \
		build/bin/indep-32/iidx-irbeat-patch.exe \
		dist/iidx/iidx-irbeat-patch-09.bat \
		dist/iidx/iidx-irbeat-patch-10.bat \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/iidx-13.zip: \
		build/bin/avs2_0-32/iidxhook2.dll \
		build/bin/indep-32/config.exe \
		build/bin/indep-32/eamio.dll \
		build/bin/indep-32/geninput.dll \
		build/bin/indep-32/iidxio.dll \
		build/bin/indep-32/vefxio.dll \
		build/bin/indep-32/inject.exe \
		dist/iidx/config.bat \
		dist/iidx/gamestart-13.bat \
		dist/iidx/iidxhook-13.conf \
		dist/iidx/vefx.txt \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/iidx-14-to-17.zip: \
		build/bin/avs2_0-32/iidxhook3.dll \
		build/bin/indep-32/config.exe \
		build/bin/indep-32/eamio.dll \
		build/bin/indep-32/geninput.dll \
		build/bin/indep-32/iidxio.dll \
		build/bin/indep-32/vefxio.dll \
		build/bin/indep-32/inject.exe \
		dist/iidx/config.bat \
		dist/iidx/gamestart-14.bat \
		dist/iidx/gamestart-15.bat \
		dist/iidx/gamestart-16.bat \
		dist/iidx/gamestart-17.bat \
		dist/iidx/iidxhook-14.conf \
		dist/iidx/iidxhook-15.conf \
		dist/iidx/iidxhook-16.conf \
		dist/iidx/iidxhook-17.conf \
		dist/iidx/vefx.txt \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/iidx-18.zip: \
		build/bin/avs2_1101-32/iidxhook4.dll \
		build/bin/avs2_1101-32/launcher.exe \
		build/bin/indep-32/config.exe \
		build/bin/indep-32/eamio.dll \
		build/bin/indep-32/geninput.dll \
		build/bin/indep-32/iidxio.dll \
		build/bin/indep-32/vefxio.dll \
		dist/iidx/config.bat \
		dist/iidx/gamestart-18.bat \
		dist/iidx/iidxhook-18.conf \
		dist/iidx/vefx.txt \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/iidx-19.zip: \
		build/bin/avs2_1304-32/iidxhook5.dll \
		build/bin/avs2_1304-32/launcher.exe \
		build/bin/indep-32/config.exe \
		build/bin/indep-32/eamio.dll \
		build/bin/indep-32/geninput.dll \
		build/bin/indep-32/iidxio.dll \
		build/bin/indep-32/vefxio.dll \
		dist/iidx/config.bat \
		dist/iidx/gamestart-19.bat \
		dist/iidx/iidxhook-19.conf \
		dist/iidx/vefx.txt \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/iidx-20.zip: \
		build/bin/avs2_1508-32/iidxhook6.dll \
		build/bin/avs2_1508-32/launcher.exe \
		build/bin/indep-32/config.exe \
		build/bin/indep-32/eamio.dll \
		build/bin/indep-32/geninput.dll \
		build/bin/indep-32/iidxio.dll \
		build/bin/indep-32/vefxio.dll \
		dist/iidx/config.bat \
		dist/iidx/gamestart-20.bat \
		dist/iidx/iidxhook-20.conf \
		dist/iidx/vefx.txt \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/iidx-21-to-24.zip: \
		build/bin/avs2_1601-32/iidxhook7.dll \
		build/bin/avs2_1601-32/launcher.exe \
		build/bin/indep-32/config.exe \
		build/bin/indep-32/eamio.dll \
		build/bin/indep-32/geninput.dll \
		build/bin/indep-32/iidxio.dll \
		build/bin/indep-32/vefxio.dll \
		dist/iidx/config.bat \
		dist/iidx/gamestart-21.bat \
		dist/iidx/gamestart-22.bat \
		dist/iidx/gamestart-23.bat \
		dist/iidx/gamestart-24.bat \
		dist/iidx/iidxhook-21.conf \
		dist/iidx/iidxhook-22.conf \
		dist/iidx/iidxhook-23.conf \
		dist/iidx/iidxhook-24.conf \
		dist/iidx/vefx.txt \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/iidx-25-to-26.zip: \
		build/bin/avs2_1700-64/iidxhook8.dll \
		build/bin/avs2_1700-64/launcher.exe \
		build/bin/indep-64/config.exe \
		build/bin/indep-64/eamio.dll \
		build/bin/indep-64/geninput.dll \
		build/bin/indep-64/iidxio.dll \
		build/bin/indep-64/vefxio.dll \
		dist/iidx/config.bat \
		dist/iidx/gamestart-25.bat \
		dist/iidx/gamestart-26.bat \
		dist/iidx/iidxhook-25.conf \
		dist/iidx/iidxhook-26.conf \
		dist/iidx/vefx.txt \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/iidx-27-to-28.zip: \
		build/bin/avs2_1700-64/iidxhook9.dll \
		build/bin/avs2_1700-64/launcher.exe \
		build/bin/indep-64/config.exe \
		build/bin/indep-64/eamio.dll \
		build/bin/indep-64/geninput.dll \
		build/bin/indep-64/iidxio.dll \
		build/bin/indep-64/vefxio.dll \
		dist/iidx/config.bat \
		dist/iidx/gamestart-27.bat \
		dist/iidx/gamestart-28.bat \
		dist/iidx/iidxhook-27.conf \
		dist/iidx/iidxhook-28.conf \
		dist/iidx/vefx.txt \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/iidx-hwio-x86.zip: \
		build/bin/indep-32/aciomgr.dll \
		build/bin/indep-32/eamio-icca.dll \
		build/bin/indep-32/iidxio-bio2.dll \
		build/bin/indep-32/iidxio-ezusb.dll \
		build/bin/indep-32/iidxio-ezusb2.dll \
		dist/iidx/iidxio-bio2.conf \
		build/bin/indep-32/vigem-iidxio.exe \
		dist/iidx/vigem-iidxio.conf \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/iidx-hwio-x64.zip: \
		build/bin/indep-64/aciomgr.dll \
		build/bin/indep-64/eamio-icca.dll \
		build/bin/indep-64/iidxio-bio2.dll \
		build/bin/indep-64/iidxio-ezusb.dll \
		build/bin/indep-64/iidxio-ezusb2.dll \
		dist/iidx/iidxio-bio2.conf \
		build/bin/indep-64/vigem-iidxio.exe \
		dist/iidx/vigem-iidxio.conf \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/jb-01.zip: \
		build/bin/avs2_803-32/jbhook1.dll \
		build/bin/indep-32/inject.exe \
		build/bin/indep-32/config.exe \
		build/bin/indep-32/eamio.dll \
		build/bin/indep-32/geninput.dll \
		build/bin/indep-32/jbio.dll \
		dist/jb/config.bat \
		dist/jb/gamestart-01.bat \
		dist/jb/jbhook-01.conf \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/jb-02.zip: \
		build/bin/avs2_1002-32/jbhook1.dll \
		build/bin/indep-32/inject.exe \
		build/bin/indep-32/config.exe \
		build/bin/indep-32/eamio.dll \
		build/bin/indep-32/geninput.dll \
		build/bin/indep-32/jbio.dll \
		dist/jb/config.bat \
		dist/jb/gamestart-02.bat \
		dist/jb/jbhook-02.conf \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/jb-03.zip: \
		build/bin/avs2_1101-32/jbhook2.dll \
		build/bin/avs2_1101-32/launcher.exe \
		build/bin/indep-32/config.exe \
		build/bin/indep-32/eamio.dll \
		build/bin/indep-32/geninput.dll \
		build/bin/indep-32/jbio.dll \
		dist/jb/config.bat \
		dist/jb/gamestart-03.bat \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/jb-04.zip: \
		build/bin/avs2_1304-32/jbhook2.dll \
		build/bin/avs2_1304-32/launcher.exe \
		build/bin/indep-32/config.exe \
		build/bin/indep-32/eamio.dll \
		build/bin/indep-32/geninput.dll \
		build/bin/indep-32/jbio.dll \
		dist/jb/config.bat \
		dist/jb/gamestart-03.bat \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/jb-05-to-07.zip: \
		build/bin/avs2_1508-32/jbhook3.dll \
		build/bin/avs2_1508-32/launcher.exe \
		build/bin/indep-32/config.exe \
		build/bin/indep-32/eamio.dll \
		build/bin/indep-32/geninput.dll \
		build/bin/indep-32/jbio.dll \
		dist/jb/config.bat \
		dist/jb/gamestart-04.bat \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/jb-08.zip: \
		build/bin/avs2_1700-32/jbhook3.dll \
		build/bin/avs2_1700-32/launcher.exe \
		build/bin/indep-32/config.exe \
		build/bin/indep-32/eamio.dll \
		build/bin/indep-32/geninput.dll \
		build/bin/indep-32/jbio.dll \
		dist/jb/config.bat \
		dist/jb/gamestart-04.bat \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/jb-hwio.zip: \
		build/bin/indep-32/aciomgr.dll \
		build/bin/indep-32/eamio-icca.dll \
		build/bin/indep-32/jbio-magicbox.dll \
		build/bin/indep-32/jbio-p4io.dll \
		dist/jb/jbio-h44b.conf \
		dist/jb/eamio-icc.conf \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/sdvx-01-to-04.zip: \
		build/bin/avs2_1508-32/launcher.exe \
		build/bin/avs2_1508-32/sdvxhook.dll \
		build/bin/indep-32/config.exe \
		build/bin/indep-32/eamio.dll \
		build/bin/indep-32/geninput.dll \
		build/bin/indep-32/sdvxio.dll \
		dist/sdvx/config.bat \
		dist/sdvx/gamestart.bat \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/sdvx-05-to-06.zip: \
		build/bin/avs2_1700-64/launcher.exe \
		build/bin/avs2_1700-64/sdvxhook2.dll \
		build/bin/indep-64/config.exe \
		build/bin/indep-64/eamio.dll \
		build/bin/indep-64/geninput.dll \
		build/bin/indep-64/sdvxio.dll \
		dist/sdvx5/config.bat \
		dist/sdvx5/gamestart.bat \
		dist/sdvx5/sdvxhook2.conf \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/sdvx-05-cn.zip: \
		build/bin/avs2_1700-64/launcher.exe \
		build/bin/avs2_1700-64/sdvxhook2-cn.dll \
		build/bin/indep-64/config.exe \
		build/bin/indep-64/eamio.dll \
		build/bin/indep-64/geninput.dll \
		build/bin/indep-64/sdvxio.dll \
		dist/sdvx5/config.bat \
		dist/sdvx5/gamestart-cn.bat \
		dist/sdvx5/sdvxhook2-cn.conf \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/sdvx-hwio-x86.zip: \
		build/bin/indep-32/aciomgr.dll \
		build/bin/indep-32/eamio-icca.dll \
		build/bin/indep-32/sdvxio-kfca.dll \
		build/bin/indep-32/sdvxio-bio2.dll \
		build/bin/indep-32/vigem-sdvxio.exe \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/sdvx-hwio-x64.zip: \
		build/bin/indep-64/aciomgr.dll \
		build/bin/indep-64/eamio-icca.dll \
		build/bin/indep-64/sdvxio-kfca.dll \
		build/bin/indep-64/sdvxio-bio2.dll \
		build/bin/indep-64/vigem-sdvxio.exe \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/ddr-11.zip: \
		build/bin/indep-32/inject.exe \
		build/bin/avs2_803-32/ddrhook1.dll \
		build/bin/avs2_803-32/unicorntail.dll \
		build/bin/indep-32/config.exe \
		build/bin/indep-32/ddrio.dll \
		build/bin/indep-32/ddrio-mm.dll \
		build/bin/indep-32/ddrio-smx.dll \
		build/bin/indep-32/eamio.dll \
		build/bin/indep-32/geninput.dll \
		dist/ddr/config.bat \
		dist/ddr/gamestart-11.bat \
		dist/ddr/gamestart-11-us.bat \
		dist/ddr/ddr-11.conf \
		dist/ddr/ddr-11-us.conf \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/ddr-12-us.zip: \
		build/bin/indep-32/inject.exe \
		build/bin/avs2_1002-32/ddrhook1.dll \
		build/bin/avs2_1002-32/unicorntail.dll \
		build/bin/indep-32/config.exe \
		build/bin/indep-32/ddrio.dll \
		build/bin/indep-32/ddrio-mm.dll \
		build/bin/indep-32/ddrio-smx.dll \
		build/bin/indep-32/eamio.dll \
		build/bin/indep-32/geninput.dll \
		dist/ddr/config.bat \
		dist/ddr/gamestart-12-us.bat \
		dist/ddr/gamestart-12-eu.bat \
		dist/ddr/ddr-12-us.conf \
		dist/ddr/ddr-12-eu.conf \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/ddr-12.zip: \
		build/bin/avs2_1002-32/launcher.exe \
		build/bin/avs2_1002-32/ddrhook2.dll \
		build/bin/avs2_1002-32/unicorntail.dll \
		build/bin/indep-32/config.exe \
		build/bin/indep-32/ddrio.dll \
		build/bin/indep-32/ddrio-mm.dll \
		build/bin/indep-32/ddrio-smx.dll \
		build/bin/indep-32/eamio.dll \
		build/bin/indep-32/geninput.dll \
		dist/ddr/config.bat \
		dist/ddr/gamestart-12.bat \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/ddr-13.zip: \
		build/bin/avs2_1304-32/launcher.exe \
		build/bin/avs2_1304-32/ddrhook2.dll \
		build/bin/avs2_1304-32/unicorntail.dll \
		build/bin/indep-32/config.exe \
		build/bin/indep-32/ddrio.dll \
		build/bin/indep-32/ddrio-mm.dll \
		build/bin/indep-32/ddrio-smx.dll \
		build/bin/indep-32/eamio.dll \
		build/bin/indep-32/geninput.dll \
		dist/ddr/config.bat \
		dist/ddr/gamestart-13.bat \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/ddr-14-to-16.zip: \
		build/bin/avs2_1508-32/launcher.exe \
		build/bin/avs2_1508-32/ddrhook2.dll \
		build/bin/avs2_1508-32/unicorntail.dll \
		build/bin/indep-32/config.exe \
		build/bin/indep-32/ddrio.dll \
		build/bin/indep-32/ddrio-mm.dll \
		build/bin/indep-32/ddrio-smx.dll \
		build/bin/indep-32/eamio.dll \
		build/bin/indep-32/geninput.dll \
		dist/ddr/config.bat \
		dist/ddr/gamestart-14.bat \
		dist/ddr/gamestart-15.bat \
		dist/ddr/gamestart-16.bat \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/ddr-16-x64.zip: \
		build/bin/avs2_1603-64/launcher.exe \
		build/bin/avs2_1603-64/ddrhook2.dll \
		build/bin/avs2_1603-64/unicorntail.dll \
		build/bin/indep-64/config.exe \
		build/bin/indep-64/ddrio.dll \
		build/bin/indep-64/ddrio-mm.dll \
		build/bin/indep-64/ddrio-smx.dll \
		build/bin/indep-64/eamio.dll \
		build/bin/indep-64/geninput.dll \
		dist/ddr/config.bat \
		dist/ddr/gamestart-16.bat \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/bst.zip: \
		build/bin/avs2_1603-64/bsthook.dll \
		build/bin/avs2_1603-64/launcher.exe \
		build/bin/indep-64/bstio.dll \
		build/bin/indep-64/config.exe \
		build/bin/indep-64/eamio.dll \
		build/bin/indep-64/geninput.dll \
		dist/bst/config.bat \
		dist/bst/gamestart1.bat \
		dist/bst/gamestart2.bat \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/doc.zip: \
		doc/ \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -r $@ $^

$(BUILDDIR)/tests.zip: \
		build/bin/indep-32/iidxhook1.dll \
		build/bin/avs2_0-32/iidxhook2.dll \
		build/bin/indep-32/cconfig-test.exe \
		build/bin/indep-32/cconfig-util-test.exe \
		build/bin/indep-32/cconfig-cmd-test.exe \
		build/bin/indep-32/d3d9hook.dll \
		build/bin/indep-32/d3d9hook-test.exe \
		build/bin/indep-32/iidxhook-util-config-eamuse-test.exe \
		build/bin/indep-32/iidxhook-util-config-gfx-test.exe \
		build/bin/indep-32/iidxhook-util-config-misc-test.exe \
		build/bin/indep-32/iidxhook-util-config-sec-test.exe \
		build/bin/indep-32/inject.exe \
		build/bin/indep-32/security-id-test.exe \
		build/bin/indep-32/security-mcode-test.exe \
		build/bin/indep-32/security-rp-test.exe \
		build/bin/indep-32/security-rp2-test.exe \
		build/bin/indep-32/security-rp3-test.exe \
		build/bin/indep-32/security-util-test.exe \
		build/bin/indep-32/util-net-test.exe \
		build/bin/indep-64/iidxhook8-config-cam-test.exe \
		dist/test/run-tests.sh \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(BUILDDIR)/bemanitools.zip: \
		$(zipdir)/bst.zip \
		$(zipdir)/ddr-11.zip \
		$(zipdir)/ddr-12.zip \
		$(zipdir)/ddr-12-us.zip \
		$(zipdir)/ddr-13.zip \
		$(zipdir)/ddr-14-to-16.zip \
		$(zipdir)/ddr-16-x64.zip \
		$(zipdir)/doc.zip \
		$(zipdir)/iidx-09-to-12.zip \
		$(zipdir)/iidx-13.zip \
		$(zipdir)/iidx-14-to-17.zip \
		$(zipdir)/iidx-18.zip \
		$(zipdir)/iidx-19.zip \
		$(zipdir)/iidx-20.zip \
		$(zipdir)/iidx-21-to-24.zip \
		$(zipdir)/iidx-25-to-26.zip \
		$(zipdir)/iidx-27-to-28.zip \
		$(zipdir)/iidx-hwio-x86.zip \
		$(zipdir)/iidx-hwio-x64.zip \
		$(zipdir)/jb-01.zip \
		$(zipdir)/jb-02.zip \
		$(zipdir)/jb-03.zip \
		$(zipdir)/jb-04.zip \
		$(zipdir)/jb-05-to-07.zip \
		$(zipdir)/jb-08.zip \
		$(zipdir)/jb-hwio.zip \
		$(zipdir)/sdvx-01-to-04.zip \
		$(zipdir)/sdvx-05-to-06.zip \
		$(zipdir)/sdvx-05-cn.zip \
		$(zipdir)/sdvx-hwio-x86.zip \
		$(zipdir)/sdvx-hwio-x64.zip \
		$(zipdir)/tools.zip \
		$(zipdir)/tools-x64.zip \
		CHANGELOG.md \
		LICENSE \
		README.md \
		version \

	$(V)echo ... $@
	$(V)zip -j $@ $^

all: $(BUILDDIR)/bemanitools.zip $(BUILDDIR)/tests.zip
