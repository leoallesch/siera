# platform.mk — ESP32 (ESP-IDF) platform recipe hooks
#
# Commands:  flash  monitor  menuconfig
# Usage:     make esp32              — configure (first time) + build
#            make esp32 flash        — flash to device
#            make esp32 monitor      — open serial monitor
#            make esp32 menuconfig   — open IDF menuconfig
#
# Variables:
#   <T>_IDF_PATH    — path to esp-idf clone        (default: ~/dev/lib/esp/esp-idf)
#   <T>_IDF_TARGET  — chip variant                  (default: esp32)
#   <T>_IDF_VENV    — IDF Python venv path          (optional; auto-sourced if absent)
#   <T>_XTENSA_BIN  — xtensa toolchain bin dir      (optional; auto-sourced if absent)
#   <T>_PORT        — serial port for flash/monitor (default: /dev/ttyUSB0)
#   <T>_CMAKE_ARGS  — extra cmake configure args
#   <T>_BUILD_ARGS  — extra cmake --build args

SIERA_PLATFORM_ESP32_CMDS         := flash monitor menuconfig set-target
SIERA_PLATFORM_ESP32_NOBUILD_CMDS := menuconfig set-target

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------
_SIERA_ESP32_IDF_PATH = $(or $($(1)_IDF_PATH),$(HOME)/dev/lib/esp/esp-idf)

# ENV prefix — direct PATH when venv known, else just IDF_PATH
define SIERA_PLATFORM_ESP32_ENV
$(if $($(1)_IDF_VENV),\
IDF_PATH=$(call _SIERA_ESP32_IDF_PATH,$(1)) IDF_TARGET=$(or $($(1)_IDF_TARGET),esp32) IDF_PYTHON_ENV_PATH=$($(1)_IDF_VENV) PATH="$($(1)_IDF_VENV)/bin:$($(1)_XTENSA_BIN):$(PATH)" ,\
IDF_PATH=$(call _SIERA_ESP32_IDF_PATH,$(1)) IDF_TARGET=$(or $($(1)_IDF_TARGET),esp32) )
endef

# Source export.sh only if IDF not already active in this shell
_SIERA_ESP32_SOURCE = $(if $($(1)_IDF_VENV),,\
if [ -z "$$$$IDF_TOOLS_EXPORT_CMD" ]; then . $(call _SIERA_ESP32_IDF_PATH,$(1))/export.sh; fi &&)

# ---------------------------------------------------------------------------
# Recipes  ($(1) = target prefix, $(2) = build dir)
# ---------------------------------------------------------------------------
# _SIERA_ESP32_IDF_PY(1,2) — idf.py with build dir, env, and required cmake defines
_SIERA_ESP32_IDF_PY = $(call _SIERA_ESP32_SOURCE,$(1)) $(call SIERA_PLATFORM_ESP32_ENV,$(1)) idf.py -B $(2) -D SIERA_PLATFORM=esp32

define SIERA_PLATFORM_ESP32_CONFIGURE
	$(call _SIERA_ESP32_IDF_PY,$(1),$(2)) \
	  $($(1)_CMAKE_ARGS) \
	  reconfigure
endef

define SIERA_PLATFORM_ESP32_BUILD
	$(call _SIERA_ESP32_IDF_PY,$(1),$(2)) build
endef

define SIERA_PLATFORM_ESP32_FLASH
	$(call _SIERA_ESP32_IDF_PY,$(1),$(2)) \
	  $(if $($(1)_PORT),-p $($(1)_PORT)) \
	  flash
endef

define SIERA_PLATFORM_ESP32_MONITOR
	$(call _SIERA_ESP32_IDF_PY,$(1),$(2)) \
	  $(if $($(1)_PORT),-p $($(1)_PORT)) \
	  monitor
endef

define SIERA_PLATFORM_ESP32_MENUCONFIG
	$(call _SIERA_ESP32_IDF_PY,$(1),$(2)) menuconfig
endef

define SIERA_PLATFORM_ESP32_SET_TARGET
	$(call _SIERA_ESP32_IDF_PY,$(1),$(2)) set-target $(or $($(1)_IDF_TARGET),esp32)
endef
