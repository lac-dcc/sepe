#!/bin/sh

set -e

if [ $# -lt 1 ]; then
	echo "usage: $0 <Regex from Regexes.toml>"
fi

if [ "$(basename "$(pwd)")" = "scripts" ]; then
	cd ..
fi

REGEX="$(grep -A1 "$1" Regexes.toml \
	| awk '/regex/ {print $NF}' \
	| tail -c +2 \
	| head -c -2 \
	| sed 's/\\\\/\\/g')"

make --silent
./bin/keysynth "$(./bin/keygen "$REGEX" | ./bin/keybuilder)"
