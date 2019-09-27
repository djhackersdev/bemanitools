# vim: noexpandtab sts=8 sw=8 ts=8

#
# Overridable variables
#

V               ?= @
BUILDDIR        ?= build

#
# Internal variables
#

depdir          := $(BUILDDIR)/dep
objdir          := $(BUILDDIR)/obj
bindir          := $(BUILDDIR)/bin

toolchain_32    := i686-w64-mingw32-
toolchain_64    := x86_64-w64-mingw32-

gitrev          := $(shell git rev-parse HEAD)
cppflags        := -I src -I src/main -I src/test -DGITREV=$(gitrev)
cflags          := -O2 -pipe -ffunction-sections -fdata-sections \
                    -Wall -std=c99
ldflags		:= -Wl,--gc-sections -static-libgcc

#
# The first target that GNU Make encounters becomes the default target.
# Define our ultimate target (`all') here, and also some helpers
#

all:

# Generate a version file to identify the build
version:
	@echo "$(gitrev)" > version

.PHONY: clean

clean:
	$(V)rm -rf $(BUILDDIR)

#
# Pull in module definitions
#

deps		:=

dlls		:=
exes		:=
imps		:=
libs		:=

avsdlls		:=
avsexes		:=

testexes    :=

include Module.mk

modules		:= $(dlls) $(exes) $(libs) $(avsdlls) $(avsexes) $(testexes)

#
# $1: Bitness
# $2: AVS2 minor version
# $3: Module
#

define t_moddefs

cppflags_$3	+= $(cppflags) -DBUILD_MODULE=$3
cflags_$3	+= $(cflags)
ldflags_$3	+= $(ldflags)
srcdir_$3	?= src/main/$3

endef

$(eval $(foreach module,$(modules),$(call t_moddefs,_,_,$(module))))

##############################################################################

define t_bitness

subdir_$1_indep	:= indep-$1
bindir_$1_indep	:= $(bindir)/$$(subdir_$1_indep)

$$(bindir_$1_indep):
	$(V)mkdir -p $$@

$$(eval $$(foreach imp,$(imps),$$(call t_import,$1,indep,$$(imp))))
$$(eval $$(foreach dll,$(dlls),$$(call t_linkdll,$1,indep,$$(dll))))
$$(eval $$(foreach exe,$(exes),$$(call t_linkexe,$1,indep,$$(exe))))
$$(eval $$(foreach lib,$(libs),$$(call t_archive,$1,indep,$$(lib))))

$$(eval $$(foreach avsver,$$(avsvers_$1),$$(call t_avsver,$1,$$(avsver))))

$$(eval $$(foreach exe,$(testexes),$$(call t_linkexe,$1,indep,$$(exe))))

endef

##############################################################################

define t_avsver

subdir_$1_$2	:= avs2_$2-$1
bindir_$1_$2	:= $(bindir)/$$(subdir_$1_$2)

$$(bindir_$1_$2):
	$(V)mkdir -p $$@

$$(eval $$(foreach imp,$(imps),$$(call t_import,$1,$2,$$(imp))))
$$(eval $$(foreach dll,$(avsdlls),$$(call t_linkdll,$1,$2,$$(dll))))
$$(eval $$(foreach exe,$(avsexes),$$(call t_linkexe,$1,$2,$$(exe))))

endef

##############################################################################

define t_compile

depdir_$1_$2_$3	:= $(depdir)/$$(subdir_$1_$2)/$3
abslib_$1_$2_$3	:= $$(libs_$3:%=$$(bindir_$1_indep)/lib%.a)
absdpl_$1_$2_$3	:= $$(deplibs_$3:%=$$(bindir_$1_$2)/lib%.a)
objdir_$1_$2_$3	:= $(objdir)/$$(subdir_$1_$2)/$3
obj_$1_$2_$3	:=	$$(src_$3:%.c=$$(objdir_$1_$2_$3)/%.o) \
			$$(rc_$3:%.rc=$$(objdir_$1_$2_$3)/%_rc.o)

deps		+= $$(src_$3:%.c=$$(depdir_$1_$2_$3)/%.d)

$$(depdir_$1_$2_$3):
	$(V)mkdir -p $$@

$$(objdir_$1_$2_$3):
	$(V)mkdir -p $$@

$$(objdir_$1_$2_$3)/%.o: $$(srcdir_$3)/%.c \
		| $$(depdir_$1_$2_$3) $$(objdir_$1_$2_$3)
	$(V)echo ... $$@
	$(V)$$(toolchain_$1)gcc $$(cflags_$3) $$(cppflags_$3) \
		-MMD -MF $$(depdir_$1_$2_$3)/$$*.d -MT $$@ -MP \
		-DAVS_VERSION=$2 -c -o $$@ $$<

$$(objdir_$1_$2_$3)/%_rc.o: $$(srcdir_$3)/%.rc
	$(V)echo ... $$@ [windres]
	$(V)$$(toolchain_$1)windres $$(cppflags_$3) $$< $$@

endef

##############################################################################

define t_archive

$(t_compile)

$$(bindir_$1_$2)/lib$3.a: $$(obj_$1_$2_$3) | $$(bindir_$1_$2)
	$(V)echo ... $$@
	$(V)$$(toolchain_$1)ar r $$@ $$^ 2> /dev/null
	$(V)$$(toolchain_$1)ranlib $$@

endef

##############################################################################

define t_linkdll

$(t_compile)

dll_$1_$2_$3	:= $$(bindir_$1_$2)/$3.dll
implib_$1_$2_$3	:= $$(bindir_$1_$2)/lib$3.a

$$(dll_$1_$2_$3) $$(implib_$1_$2_$3):	$$(obj_$1_$2_$3) $$(abslib_$1_$2_$3) \
					$$(absdpl_$1_$2_$3) \
					$$(srcdir_$3)/$3.def | $$(bindir_$1_$2)
	$(V)echo ... $$(dll_$1_$2_$3)
	$(V)$$(toolchain_$1)gcc -shared \
		-o $$(dll_$1_$2_$3) -Wl,--out-implib,$$(implib_$1_$2_$3) \
		$$^ $$(ldflags_$3)
	$(V)$$(toolchain_$1)strip $$(dll_$1_$2_$3)
	$(V)$$(toolchain_$1)ranlib $$(implib_$1_$2_$3)

endef

##############################################################################

define t_linkexe

$(t_compile)

exe_$1_$2_$3	:= $$(bindir_$1_$2)/$3.exe

$$(exe_$1_$2_$3): $$(obj_$1_$2_$3) $$(abslib_$1_$2_$3) $$(absdpl_$1_$2_$3) \
		| $$(bindir_$1_$2)
	$(V)echo ... $$@
	$(V)$$(toolchain_$1)gcc -o $$@ $$^ $$(ldflags_$3)
	$(V)$$(toolchain_$1)strip $$@

endef

##############################################################################

define t_import

impdef_$1_$2_$3	?= src/imports/import_$1_$2_$3.def

$$(bindir_$1_$2)/lib$3.a: $$(impdef_$1_$2_$3) | $$(bindir_$1_$2)
	$(V)echo ... $$@ [dlltool]
	$(V)$$(toolchain_$1)dlltool -l $$@ -d $$<

endef

##############################################################################

$(eval $(foreach bitness,32 64,$(call t_bitness,$(bitness))))

#
# Pull in GCC-generated dependency files
#

-include $(deps)

