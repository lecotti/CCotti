# Author: Nicolas Gabriel Cotti (ngcotti@gmail.com)

#------------------------------------------------------------------------------
# Makefile Initialization
#------------------------------------------------------------------------------
SHELL=/bin/bash
.ONESHELL:
.POSIX:
.EXPORT_ALL_VARIABLES:
.DELETE_ON_ERROR:
.SILENT:
.DEFAULT_GOAL := help

ifndef VERBOSE
MAKEFLAGS += --no-print-directory
endif

#------------------------------------------------------------------------------
# User targets
#------------------------------------------------------------------------------
.PHONY: compile
compile: ## Compile binaries
	cmake --build ./build

.PHONY: exe
exe: ./build/user_code	## Execute code
	./build/user_code

.PHONY: test
test: compile ## Test and compile code, with added verbosity.
	ctest --verbose --test-dir ./build

.PHONY: clear, clean
clean: ## Erase contents of build directory.
	cmake --build ./build --target clean

clear: clean ## Same as clean.

.PHONY: help
help: ## Display this message.
	grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-10s\033[0m %s\n", $$1, $$2}'