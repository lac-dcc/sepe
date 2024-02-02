#!/bin/sh

set -e

if [ $# -lt 1 ]; then
	echo "usage: $0 <regex>"
fi

if [ "$(basename "$(pwd)")" = "scripts" ]; then
	cd ..
fi

make --silent -j"$(nproc)" bin/keygen bin/keybuilder bin/keysynth
./bin/keysynth "$(./bin/keygen "$1" | ./bin/keybuilder)"
