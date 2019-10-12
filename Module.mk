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
# 10:   Patch 4:    DanceDanceRevolution X2
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
#                   SOUND VOLTEX (all known versions)
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

avsvers_32      := 1700 1603 1601 1508 1403 1304 1101 803 0
avsvers_64      := 1700 1603 1601 1508

imps            += avs avs-ea3

include src/main/aciodrv/Module.mk
include src/main/acioemu/Module.mk
include src/main/aciotest/Module.mk
include src/main/bio2emu/Module.mk
include src/main/bsthook/Module.mk
include src/main/bstio/Module.mk
include src/main/cconfig/Module.mk
include src/main/config/Module.mk
include src/main/ddrhook/Module.mk
include src/main/ddrio/Module.mk
include src/main/ddrio-smx/Module.mk
include src/main/ddrio-mm/Module.mk
include src/main/eamio/Module.mk
include src/main/eamio-icca/Module.mk
include src/main/eamiotest/Module.mk
include src/main/ezusb/Module.mk
include src/main/ezusb-emu/Module.mk
include src/main/ezusb-iidx/Module.mk
include src/main/ezusb-iidx-fpga-flash/Module.mk
include src/main/ezusb-iidx-sram-flash/Module.mk
include src/main/ezusb-tool/Module.mk
include src/main/ezusb2/Module.mk
include src/main/ezusb2-dbg-hook/Module.mk
include src/main/ezusb2-emu/Module.mk
include src/main/ezusb2-iidx/Module.mk
include src/main/ezusb2-iidx-emu/Module.mk
include src/main/ezusb2-tool/Module.mk
include src/main/ezusb-iidx-emu/Module.mk
include src/main/geninput/Module.mk
include src/main/hook/Module.mk
include src/main/hooklib/Module.mk
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
include src/main/iidxio/Module.mk
include src/main/iidxio-ezusb/Module.mk
include src/main/iidxio-ezusb2/Module.mk
include src/main/iidxiotest/Module.mk
include src/main/inject/Module.mk
include src/main/jbio/Module.mk
include src/main/jbhook/Module.mk
include src/main/jbhook1/Module.mk
include src/main/launcher/Module.mk
include src/main/mempatch-hook/Module.mk
include src/main/mm/Module.mk
include src/main/p3io/Module.mk
include src/main/p3ioemu/Module.mk
include src/main/p4ioemu/Module.mk
include src/main/pcbidgen/Module.mk
include src/main/sdvxhook/Module.mk
include src/main/sdvxio/Module.mk
include src/main/sdvxio-kfca/Module.mk
include src/main/security/Module.mk
include src/main/unicorntail/Module.mk
include src/main/util/Module.mk
include src/main/vefxio/Module.mk

include src/test/cconfig/Module.mk
include src/test/d3d9hook/Module.mk
include src/test/iidxhook-util/Module.mk
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
		build/bin/indep-32/iidx-ezusb-exit-hook.dll \
		build/bin/indep-32/iidx-ezusb2-exit-hook.dll \
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
		build/bin/indep-64/eamiotest.exe \
		build/bin/indep-64/iidxiotest.exe \
		build/bin/indep-64/iidx-ezusb-exit-hook.dll \
		build/bin/indep-64/iidx-ezusb2-exit-hook.dll \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/src.zip: .git/HEAD  $(zipdir)/
	$(V)echo ... $@
	$(V)git archive -o $@ HEAD

$(zipdir)/iidx-09-to-12.zip: \
		build/bin/indep-32/iidxhook1.dll \
		build/bin/indep-32/config.exe \
		build/bin/indep-32/eamio.dll \
		build/bin/indep-32/eamio-icca.dll \
		build/bin/indep-32/geninput.dll \
		build/bin/indep-32/iidxio.dll \
		build/bin/indep-32/iidxio-ezusb.dll \
		build/bin/indep-32/iidxio-ezusb2.dll \
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
		build/bin/indep-32/eamio-icca.dll \
		build/bin/indep-32/geninput.dll \
		build/bin/indep-32/iidxio.dll \
		build/bin/indep-32/iidxio-ezusb.dll \
		build/bin/indep-32/iidxio-ezusb2.dll \
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
		build/bin/indep-32/eamio-icca.dll \
		build/bin/indep-32/geninput.dll \
		build/bin/indep-32/iidxio.dll \
		build/bin/indep-32/iidxio-ezusb.dll \
		build/bin/indep-32/iidxio-ezusb2.dll \
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
		build/bin/indep-32/eamio-icca.dll \
		build/bin/indep-32/geninput.dll \
		build/bin/indep-32/iidxio.dll \
		build/bin/indep-32/iidxio-ezusb.dll \
		build/bin/indep-32/iidxio-ezusb2.dll \
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
		build/bin/indep-32/eamio-icca.dll \
		build/bin/indep-32/geninput.dll \
		build/bin/indep-32/iidxio.dll \
		build/bin/indep-32/iidxio-ezusb.dll \
		build/bin/indep-32/iidxio-ezusb2.dll \
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
		build/bin/indep-32/eamio-icca.dll \
		build/bin/indep-32/geninput.dll \
		build/bin/indep-32/iidxio.dll \
		build/bin/indep-32/iidxio-ezusb.dll \
		build/bin/indep-32/iidxio-ezusb2.dll \
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
		build/bin/indep-32/eamio-icca.dll \
		build/bin/indep-32/geninput.dll \
		build/bin/indep-32/iidxio.dll \
		build/bin/indep-32/iidxio-ezusb.dll \
		build/bin/indep-32/iidxio-ezusb2.dll \
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

$(zipdir)/iidx-25.zip: \
		build/bin/avs2_1700-64/iidxhook8.dll \
		build/bin/avs2_1700-64/launcher.exe \
		build/bin/indep-64/config.exe \
		build/bin/indep-64/eamio.dll \
		build/bin/indep-64/eamio-icca.dll \
		build/bin/indep-64/geninput.dll \
		build/bin/indep-64/iidxio.dll \
		build/bin/indep-64/iidxio-ezusb.dll \
        build/bin/indep-64/iidxio-ezusb2.dll \
		build/bin/indep-64/vefxio.dll \
		dist/iidx/config.bat \
		dist/iidx/gamestart-25.bat \
		dist/iidx/iidxhook-25.conf \
		dist/iidx/vefx.txt \
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

$(zipdir)/jb-05-to-07.zip: \
		build/bin/avs2_1508-32/jbhook.dll \
		build/bin/avs2_1508-32/launcher.exe \
		build/bin/indep-32/config.exe \
		build/bin/indep-32/eamio.dll \
		build/bin/indep-32/geninput.dll \
		build/bin/indep-32/jbio.dll \
		dist/jb/config.bat \
		dist/jb/gamestart.bat \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/jb-08.zip: \
		build/bin/avs2_1700-32/jbhook.dll \
		build/bin/avs2_1700-32/launcher.exe \
		build/bin/indep-32/config.exe \
		build/bin/indep-32/eamio.dll \
		build/bin/indep-32/geninput.dll \
		build/bin/indep-32/jbio.dll \
		dist/jb/config.bat \
		dist/jb/gamestart.bat \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/sdvx.zip: \
		build/bin/avs2_1508-32/launcher.exe \
		build/bin/avs2_1508-32/sdvxhook.dll \
		build/bin/indep-32/config.exe \
		build/bin/indep-32/eamio.dll \
		build/bin/indep-32/geninput.dll \
		build/bin/indep-32/sdvxio.dll \
		build/bin/indep-32/sdvxio-kfca.dll \
		dist/sdvx/config.bat \
		dist/sdvx/gamestart.bat \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(zipdir)/ddr-12-to-16.zip: \
		build/bin/avs2_1508-32/launcher.exe \
		build/bin/avs2_1508-32/ddrhook.dll \
		build/bin/avs2_1508-32/unicorntail.dll \
		build/bin/indep-32/config.exe \
		build/bin/indep-32/ddrio.dll \
		build/bin/indep-32/ddrio-mm.dll \
		build/bin/indep-32/ddrio-smx.dll \
		build/bin/indep-32/eamio.dll \
		build/bin/indep-32/geninput.dll \
		dist/ddr/config.bat \
		dist/ddr/gamestart-12.bat \
		dist/ddr/gamestart-13.bat \
		dist/ddr/gamestart-14.bat \
		dist/ddr/gamestart-15.bat \
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
		doc/iidxhook \
		doc/jbhook \
		doc/tools \
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
		dist/test/run-tests.sh \
		| $(zipdir)/
	$(V)echo ... $@
	$(V)zip -j $@ $^

$(BUILDDIR)/bemanitools.zip: \
		$(zipdir)/bst.zip \
		$(zipdir)/ddr-12-to-16.zip \
		$(zipdir)/doc.zip \
		$(zipdir)/iidx-09-to-12.zip \
		$(zipdir)/iidx-13.zip \
		$(zipdir)/iidx-14-to-17.zip \
		$(zipdir)/iidx-18.zip \
		$(zipdir)/iidx-19.zip \
		$(zipdir)/iidx-20.zip \
		$(zipdir)/iidx-21-to-24.zip \
		$(zipdir)/iidx-25.zip \
		$(zipdir)/jb-01.zip \
		$(zipdir)/jb-05-to-07.zip \
		$(zipdir)/jb-08.zip \
		$(zipdir)/src.zip \
		$(zipdir)/sdvx.zip \
		$(zipdir)/tools.zip \
		$(zipdir)/tools-x64.zip \
		CHANGELOG.md \
		LICENSE \
		README.md \
		version \

	$(V)echo ... $@
	$(V)zip -j $@ $^

all: $(BUILDDIR)/bemanitools.zip $(BUILDDIR)/tests.zip
