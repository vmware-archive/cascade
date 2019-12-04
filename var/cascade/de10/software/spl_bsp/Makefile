###############################################################################
#
# Altera SoC EDS Preloader/UBoot Build System
#
# Copyright (c) 2013,2014 Altera Corporation
# All Rights Reserved
#
###############################################################################


###############################################################################
#
# Generated Variables
#

PRELOADER_SETTINGS_DIR := ../../hps_isw_handoff/soc_system_hps_0

TGZ := $(SOCEDS_DEST_ROOT)/host_tools/altera/preloader/uboot-socfpga.tar.gz

CROSS_COMPILE := arm-altera-eabi-

DEVICE_FAMILY := cyclone5

###############################################################################


###############################################################################
#
# Derived Variables
#

MAKE_ARGS += CROSS_COMPILE=$(CROSS_COMPILE)

PRELOADER_SRC_DIR := $(patsubst %.tar.gz,%,$(shell basename $(TGZ)))

PRELOADER_UPDATE_DIR := $(PRELOADER_SRC_DIR)/board/altera/socfpga

SOCFPGA_BOARD_CONFIG := socfpga_$(DEVICE_FAMILY)_config

###############################################################################


###############################################################################
#
# Tools
#

CAT := cat
CHMOD := chmod
CP := cp -rf
DIFF := diff
ECHO := echo
PATCH := patch
MKDIR := mkdir -p
RM := rm -rf
TOUCH := touch
UNTAR := tar zxf

###############################################################################


###############################################################################
#
# Helper Variables and Functions
#

STAMP_DIR ?= $(PRELOADER_SRC_DIR)

define stamp
@$(MKDIR) $(@D)
@$(TOUCH) $@
endef

define untar_recipe
$(UNTAR) $(if $1,$1,$(if $<,$<,$(error ERROR: no input provided to gnu make function untar_recipe)))
endef

# Stamps
UNTAR_SRC := $(STAMP_DIR)/.untar
CONFIG := $(STAMP_DIR)/.config
PATCH_APPLY := $(STAMP_DIR)/.patch

###############################################################################


###############################################################################
#
# Windows Support
#
#
HOSTOS := $(shell uname -o 2>/dev/null | tr [:upper:] [:lower:])

ifeq ($(HOSTOS),cygwin)

# When using UBoot build system on Windows it's good idea to use cygwin's GNU make
MAKE := $(shell cygpath -m "/bin/make")
MAKE_ARGS += MAKE=/bin/make

CYGPATH := $(shell cygpath -m "$(shell which cygpath)")
MAKE_ARGS += CYGPATH=$(CYGPATH)

UNAME_M := $(shell uname -m)
ifeq ($(UNAME_M),x86_64)
HOST_CROSS_COMPILE := x86_64-w64-mingw32-
else
HOST_CROSS_COMPILE := i686-pc-mingw32-
endif

MAKE_ARGS += HOSTCC=$(HOST_CROSS_COMPILE)gcc HOSTSTRIP=$(HOST_CROSS_COMPILE)strip

# Under cygwin, overload the untar_recipe function to use unix stype paths. This is required for cygwin tar
define untar_recipe
$(UNTAR) $(shell cygpath --unix "$(if $1,$1,$(if $<,$<,$(error ERROR: no input provided to gnu make function untar_recipe)))")
endef

else # if HOSTOS != cygwin

ifdef WINDIR
$(error ERROR: Windows build of preloader requires cygwin build environment. Ensure this makefile is executed from the SoC EDS Command Shell)
endif
ifdef windir
$(error ERROR: Windows build of preloader requires cygwin build environment. Ensure this makefile is executed from the SoC EDS Command Shell)
endif

endif # HOSTOS == cygwin
###############################################################################


###############################################################################
#
# Source Files
#

SDRAM_SOURCE_FILES := \
	alt_types.h \
	sdram_io.h \
	sequencer_auto_ac_init.c \
	sequencer_auto.h \
	sequencer_auto_inst_init.c \
	sequencer.c \
	sequencer_defines.h \
	sequencer.h \
	system.h \
	tclrpt.c \
	tclrpt.h

GENERATED_SOURCE_FILES := \
	build.h \
	iocsr_config_$(DEVICE_FAMILY).c \
	iocsr_config_$(DEVICE_FAMILY).h \
	reset_config.h \
	pll_config.h \
	pinmux_config_$(DEVICE_FAMILY).c \
	pinmux_config.h \
	sdram/sdram_config.h


UPDATE_SDRAM_SOURCE_FILES := $(patsubst %,$(PRELOADER_UPDATE_DIR)/sdram/%,$(SDRAM_SOURCE_FILES))

UPDATE_GENERATED_SOURCE_FILES := $(patsubst %,$(PRELOADER_UPDATE_DIR)/%,$(GENERATED_SOURCE_FILES))

###############################################################################


###############################################################################
#
# Main build targets 
#

PRELOADER.BINARY := $(PRELOADER_SRC_DIR)/spl/u-boot-spl.bin
PRELOADER.MKPIMAGE_BINARY := preloader-mkpimage.bin
UBOOT.BINARY := $(PRELOADER_SRC_DIR)/u-boot.bin

.PHONY: all
all: spl mkpimage-spl

.PHONY: uboot
uboot: $(UBOOT.BINARY)

$(UBOOT.BINARY): $(CONFIG)
	$(MAKE) $(MAKE_ARGS) -C $(PRELOADER_SRC_DIR)

.PHONY: spl
spl: $(PRELOADER.BINARY)

$(PRELOADER.BINARY): $(CONFIG)
	$(MAKE) $(MAKE_ARGS) -C $(PRELOADER_SRC_DIR) spl/u-boot-spl.bin

.PHONY: mkpimage-spl
mkpimage-spl: $(PRELOADER.MKPIMAGE_BINARY)

$(PRELOADER.MKPIMAGE_BINARY): $(PRELOADER.BINARY)
	mkpimage --header-version 0 -o $@ $< $< $< $<

.PHONY: tools
tools: $(CONFIG)
	$(MAKE) $(MAKE_ARGS) -C $(PRELOADER_SRC_DIR) tools


###############################################################################


###############################################################################
#
# Untar, Update, & Configure Preloader Source
#


#####
# Untar
.PHONY: src
src: $(UNTAR_SRC)

$(UNTAR_SRC): $(TGZ)
	@$(RM) $(PRELOADER_SRC_DIR)
	$(untar_recipe)
	@$(CHMOD) -R 755 $(PRELOADER_SRC_DIR)
	$(stamp)

#####
# Update
.PHONY: update-src
update-src: $(UPDATE_SRC)

UPDATE_SRC += $(UPDATE_GENERATED_SOURCE_FILES) $(UPDATE_SDRAM_SOURCE_FILES)

$(UPDATE_SDRAM_SOURCE_FILES): $(PRELOADER_UPDATE_DIR)/sdram/%: $(PRELOADER_SETTINGS_DIR)/% $(UNTAR_SRC) $(PATCH.APPLY_TARGETS)
	@$(MKDIR) $(@D)
	@$(CP) -v $< $@

$(UPDATE_GENERATED_SOURCE_FILES): $(PRELOADER_UPDATE_DIR)/%: generated/% $(UNTAR_SRC) $(PATCH.APPLY_TARGETS)
	@$(MKDIR) $(@D)
	@$(CP) -v $< $@

#####
# Configure
.PHONY: config
config: $(CONFIG)

$(CONFIG): $(UPDATE_SRC) $(UNTAR_SRC) $(PATCH_APPLY)
	$(MAKE) $(MAKE_ARGS) -C $(PRELOADER_SRC_DIR) $(SOCFPGA_BOARD_CONFIG)
	$(stamp)

###############################################################################


###############################################################################
#
# Cleaning up
#

.PHONY: clean
clean:
ifneq ($(wildcard $(PRELOADER_SRC_DIR)),)
	$(MAKE) $(MAKE_ARGS) -C $(PRELOADER_SRC_DIR) mrproper
endif
	$(RM) $(PRELOADER.MKPIMAGE_BINARY) $(CONFIG)

.PHONY: clean-all
clean-all:
	$(RM) $(PRELOADER_SRC_DIR) $(PRELOADER.MKPIMAGE_BINARY) $(CONFIG) $(PATCH_APPLY) $(UNTAR_SRC)

###############################################################################


###############################################################################
#
# Applying patch files
#

# GNU MAKE >= 3.81 is required to apply patch files correctly
.SECONDEXPANSION:

# Patch files are discovered in current directory and in the directory adjacent
# to the tarball (TGZ) directory

PATCH.FILES := $(strip \
	$(sort $(wildcard $(patsubst %.tar.gz,%.patch,$(TGZ))/*.patch)) \
	$(sort $(wildcard $(patsubst %.tar.gz,%.patch,$(TGZ))/$(HOSTOS)/*.patch)) \
	$(sort $(wildcard $(abspath .)/*.patch)) \
	$(EXTRA_PATCH_FILES))

PATCH.APPLY_TARGETS := $(strip $(foreach patchfile,$(PATCH.FILES), \
 $(eval patchfile_target := $(notdir $(basename $(patchfile)))) \
 $(eval $(patchfile_target).PATCH_FILE := $(patchfile)) \
 $(PRELOADER_SRC_DIR)/.applypatch.$(patchfile_target) \
))

.PHONY: patch-apply
patch-apply: $(PATCH_APPLY)

$(PATCH_APPLY): $(PATCH.APPLY_TARGETS)
	$(stamp)

$(PATCH.APPLY_TARGETS): $(PRELOADER_SRC_DIR)/.applypatch.%: $$(%.PATCH_FILE) $(UNTAR_SRC)
	@$(ECHO) Applying Patch: $<
	$(PATCH) -p1 --directory=$(PRELOADER_SRC_DIR) --input=$<
	$(stamp)

###############################################################################


###############################################################################
#
# Creating a patch file
#

PATCH.USER_FILE := user.patch

.PHONY: patch-create
patch-create: $(if $(PATCH.SKIP_CLEAN),,clean)
ifeq ($(wildcard $(PRELOADER_SRC_DIR).orig),)
	$(error ERROR: $(PRELOADER_SRC_DIR).orig does not exist)
endif
	$(DIFF) -rupN $(PRELOADER_SRC_DIR).orig/ $(PRELOADER_SRC_DIR)/ > $(PATCH.USER_FILE) || true
ifeq ($(HOSTOS),cygwin)
	dos2unix $(PATCH.USER_FILE)
endif
	$(CAT) $(PATCH.USER_FILE)

###############################################################################


