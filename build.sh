#!/usr/bin/env bash
set -euo pipefail

target="Karbon"
if [[ "${1:-}" == "editor" ]]; then
	target="karbon_editor"
fi

cmake . && make && "./build/${target}" #Just a quick lil one-liner script to make life easier in console :p