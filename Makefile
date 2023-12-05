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

#------------------------------------------------------------------------------
# User targets
#------------------------------------------------------------------------------
.PHONY: compile
compile: ## Compile binaries
	cd ./build
	cmake ..
	cd ..
	cmake --build ./build

.PHONY: test
test: compile ## Test and compile code, with added verbosity.
	ctest --verbose --test-dir ./build

.PHONY: clear, clean
clean: ## Erase contents of build directory.
	cd build
	rm -rf *
	cd ..

clear: clean ## Same as clean.

.PHONY: help
help: ## Display this message.
	grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-10s\033[0m %s\n", $$1, $$2}'