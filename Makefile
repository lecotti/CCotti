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

user := debian
password := temppwd
ip := 192.168.7.2
install_path := /home/${user}/server/

#------------------------------------------------------------------------------
# User targets
#------------------------------------------------------------------------------
.PHONY: compile
compile: ## Compile binaries
	cd ./build
	cmake ..
	cd ..
	cmake --build ./build

.PHONY: exe
exe: ./build/cotti_server	## Execute code
	./build/cotti_server

.PHONY: test
test: compile ## Test and compile code, with added verbosity.
	ctest --verbose --test-dir ./build

.PHONY: clear, clean
clean: ## Erase contents of build directory.
	cd build
	rm -rf *
	cd ..

clear: clean ## Same as clean.

.PHONY: bbb_compile
bbb_compile: clean bbb_copy	## Compile the server on the BeagleBone Black
	echo "Compiling..."
	sshpass -p "${password}" ssh -q "${user}@${ip}" \
    	"${install_path}/install_on_beagle.sh"


.PHONY: bbb_copy
bbb_copy:	## Copy all files to the beaglebone
	echo "Copying all files to the beaglebone..."
	for file in *; do
		sshpass -p "${password}" scp -q -r "$${file}" "${user}@${ip}:${install_path}"
	done

	sshpass -p "${password}" ssh -q "${user}@${ip}" \
		"mv ${install_path}config.cfg /home/${user}/config.cfg"

.PHONY: bbb_exe
bbb_exe:	## Execute the server on the BeagleBone Black
	if [ -f "/temp/cotti_server_pid" ]; then
		echo "Server already running"
	else
		sshpass -p "${password}" ssh -q "${user}@${ip}" \
        	"/home/debian/server/build/cotti_server"
	fi

.PHONY: bbb_server_config
bbb_server_config:	## Change server configuration file, and send SIGUSR1 to the server
	if [ -f "/temp/cotti_server_pid" ]; then
		sshpass -p "${password}" ssh -q -t "${user}@${ip}" \
        	"nano /home/${user}/config.cfg && kill -s SIGUSR1 ${cat /temp/cotti_server_pid}"
	else
		echo "The server is not running!"
	fi

.PHONY: help
help: ## Display this message.
	grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-10s\033[0m %s\n", $$1, $$2}'