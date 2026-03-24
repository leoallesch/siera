# platform.mk — POSIX (host) platform recipe hooks
#
# Commands:  run
# Usage:     make posix        — configure (first time) + build
#            make posix run    — build then run the executable
#
# Variables:
#   <T>_CMAKE_ARGS  — extra cmake configure args
#   <T>_BUILD_ARGS  — extra cmake --build args
#   <T>_EXECUTABLE  — name of the built binary (required for 'run')
#   <T>_RUN_ARGS    — arguments forwarded to the executable on run

SIERA_PLATFORM_POSIX_CMDS := run

# $(1) = target prefix, $(2) = build dir
define SIERA_PLATFORM_POSIX_CONFIGURE
	cmake -B $(2) -DSIERA_PLATFORM=posix $($(1)_CMAKE_ARGS)
endef

define SIERA_PLATFORM_POSIX_BUILD
	cmake --build $(2) -- -j$$(shell nproc) $($(1)_BUILD_ARGS)
endef

define SIERA_PLATFORM_POSIX_RUN
	$(if $($(1)_EXECUTABLE),,$(error $(1)_EXECUTABLE is not set — set it to the name of your cmake executable target))
	$(2)/$($(1)_EXECUTABLE) $($(1)_RUN_ARGS)
endef
