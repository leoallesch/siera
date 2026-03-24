# siera.mk — convenience build interface for SIERA multi-platform projects
#
# USAGE
# -----
#   SIERA_ROOT ?= path/to/siera
#   BUILD_DIR  ?= build
#
#   POSIX_PLATFORM := posix
#
#   ESP32_PLATFORM     := esp32
#   ESP32_IDF_TARGET   ?= esp32
#   ESP32_PORT         ?= /dev/ttyUSB0
#
#   include $(SIERA_ROOT)/siera.mk
#
# INVOKING TARGETS
# ----------------
#   make <T>                — configure (first time) + build
#   make <T> <cmd>          — run a platform command after ensuring built
#                             e.g.  make esp32 flash
#                                   make esp32 monitor
#                                   make esp32 menuconfig
#                                   make posix run
#   make <T> configure      — force reconfigure
#   make <T> clean          — remove build dir
#   make all                — build all targets
#   make clean              — clean all targets
#
# How <T> <cmd> works:
#   Both words appear in $(MAKECMDGOALS).  siera.mk registers each known
#   command name as a phony that dispatches to the platform implementation
#   for whichever target is also present in $(MAKECMDGOALS).
#   If a command is invoked without a target (e.g. bare `make flash`) it
#   errors with a helpful message.
#
# PER-TARGET VARIABLES  (PREFIX = upper-case, e.g. ESP32)
# --------------------------------------------------------
# Required:
#   <PREFIX>_PLATFORM    — platform name (must match cmake/platforms/<name>/)
#
# Optional (any platform):
#   <PREFIX>_BUILD_DIR   — build dir override  (default: $(BUILD_DIR)/<lower>)
#   <PREFIX>_CMAKE_ARGS  — extra cmake configure args
#   <PREFIX>_BUILD_ARGS  — extra cmake --build args
#
# Optional (esp32):
#   <PREFIX>_IDF_PATH    — path to esp-idf clone
#   <PREFIX>_IDF_TARGET  — chip variant (esp32, esp32s3, …)
#   <PREFIX>_IDF_VENV    — IDF Python venv path
#   <PREFIX>_XTENSA_BIN  — xtensa toolchain bin dir
#   <PREFIX>_PORT        — serial port for flash/monitor

SIERA_ROOT ?= .
BUILD_DIR  ?= build

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------
_siera_lc      = $(shell echo '$(1)' | tr 'A-Z' 'a-z')
_siera_uc      = $(shell echo '$(1)' | tr 'a-z' 'A-Z')
_siera_uc_safe = $(shell echo '$(1)' | tr 'a-z-' 'A-Z_')

_siera_all_lower    :=
_siera_loaded_plats :=
_siera_all_cmds     :=   # union of all command names across all platforms

define _siera_include_platform
$(if $(filter $(1),$(_siera_loaded_plats)),,\
  $(eval _siera_loaded_plats += $(1))\
  $(eval include $(SIERA_ROOT)/cmake/platforms/$(1)/platform.mk))
endef

# ---------------------------------------------------------------------------
# _siera_gen_target
# $(1) UPPER prefix  $(2) platform  $(3) build dir  $(4) lower label  $(5) UPPER platform
# ---------------------------------------------------------------------------
define _siera_gen_target

# --- configure (real file target — skipped if build.ninja exists) ---------
$(3)/build.ninja:
	$(call SIERA_PLATFORM_$(5)_CONFIGURE,$(1),$(3))

# --- build (default action for this target) --------------------------------
.PHONY: $(4)
$(4): $(3)/build.ninja
	$(call SIERA_PLATFORM_$(5)_BUILD,$(1),$(3))

# --- per-target clean / configure ------------------------------------------
.PHONY: _$(4)_clean _$(4)_configure
_$(4)_clean:
	rm -rf $(3)
_$(4)_configure: FORCE
	$(call SIERA_PLATFORM_$(5)_CONFIGURE,$(1),$(3))

# --- register platform commands and wire dispatch --------------------------
$(foreach _cmd,$(SIERA_PLATFORM_$(5)_CMDS),\
  $(eval _siera_all_cmds += $(_cmd))\
  $(eval _siera_$(4)_cmds += $(_cmd))\
  $(eval _siera_$(4)_bdir  := $(3))\
  $(eval _siera_$(4)_pfx   := $(1))\
  $(eval _siera_$(4)_uplat := $(5))\
)

endef

# ---------------------------------------------------------------------------
# _siera_gen_cmd  — emit the phony dispatch rule for one command name
# Called after all targets are registered so we know the full target list.
# $(1) = command name  e.g. flash
# ---------------------------------------------------------------------------
define _siera_gen_cmd

.PHONY: $(1)
$(1): $(foreach _t,$(_siera_all_lower),$(if $(filter $(1),$(_siera_$(_t)_cmds)),$(if $(filter $(_t),$(MAKECMDGOALS)),$(if $(filter $(1),$(SIERA_PLATFORM_$(_siera_$(_t)_uplat)_NOBUILD_CMDS)),,$(_t)))))
	@$(foreach _t,$(_siera_all_lower),\
	  $(if $(filter $(1),$(_siera_$(_t)_cmds)),\
	    $(if $(filter $(_t),$(MAKECMDGOALS)),\
	      exit 0 ;))) \
	  $(if $(filter 1,$(words $(foreach _t,$(_siera_all_lower),$(if $(filter $(1),$(_siera_$(_t)_cmds)),$(_t))))),\
	    exit 0 ,\
	    echo "error: '$(1)' requires a target — try: make <target> $(1)" ; exit 1)
	$(foreach _t,$(_siera_all_lower),\
	  $(if $(filter $(1),$(_siera_$(_t)_cmds)),\
	    $(if $(or $(filter $(_t),$(MAKECMDGOALS)),$(filter 1,$(words $(foreach _t2,$(_siera_all_lower),$(if $(filter $(1),$(_siera_$(_t2)_cmds)),$(_t2)))))),\
	      $(call SIERA_PLATFORM_$(_siera_$(_t)_uplat)_$(call _siera_uc_safe,$(1)),$(_siera_$(_t)_pfx),$(_siera_$(_t)_bdir)))))

endef

# ---------------------------------------------------------------------------
# Auto-discover *_PLATFORM variables and register targets
# ---------------------------------------------------------------------------
$(foreach _v,$(filter %_PLATFORM,$(.VARIABLES)),\
  $(eval _pfx := $(patsubst %_PLATFORM,%,$(_v)))\
  $(if $(filter-out _siera% SIERA_ROOT SIERA_BUILD,$(_pfx)),\
    $(eval _plat  := $($(_v)))\
    $(eval _lower := $(call _siera_lc,$(_pfx)))\
    $(eval _bdir  := $(or $($(_pfx)_BUILD_DIR),$(BUILD_DIR)/$(_lower)))\
    $(eval _uplat := $(call _siera_uc,$(_plat)))\
    $(call _siera_include_platform,$(_plat))\
    $(eval $(call _siera_gen_target,$(_pfx),$(_plat),$(_bdir),$(_lower),$(_uplat)))\
    $(eval _siera_all_lower += $(_lower))\
  )\
)

# Emit one dispatch rule per unique command name (deduplicated)
$(foreach _cmd,$(sort $(_siera_all_cmds)),\
  $(eval $(call _siera_gen_cmd,$(_cmd)))\
)

# 'clean' and 'configure' dispatch to the target(s) named alongside them,
# or to all targets when invoked alone.
.PHONY: configure clean

configure:
	@$(foreach _t,$(_siera_all_lower),\
	  $(if $(filter $(_t),$(MAKECMDGOALS)),\
	    $(MAKE) _$(_t)_configure ;)) true

clean:
	@$(if $(filter $(_siera_all_lower),$(MAKECMDGOALS)),\
	  $(foreach _t,$(_siera_all_lower),\
	    $(if $(filter $(_t),$(MAKECMDGOALS)),\
	      $(MAKE) _$(_t)_clean ;)),\
	  $(foreach _t,$(_siera_all_lower), $(MAKE) _$(_t)_clean ;)) true

# ---------------------------------------------------------------------------
# Aggregate
# ---------------------------------------------------------------------------
.PHONY: all FORCE

all: $(_siera_all_lower)

FORCE:
