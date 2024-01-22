#!/bin/sh

set -e

if [ "$(basename "$(pwd)")" = "scripts" ]; then
	cd ..
fi

if [ ! -d output ]; then
	mkdir output
fi

make

NUM_KEYS="100000 250000 500000 1000000"
NUM_OPS="10"
REGEXES="$(sed  -n 's/^\[\(.*\)\]/\1/p' Regexes.toml)"
REPETITIONS="10"

for REGEX in $REGEXES; do
	COUNT=0
	for NUM_KEY in $NUM_KEYS; do
		for NUM_OP in $NUM_OPS; do
			./bin/bench-runner \
				--verbose \
				--operations "$NUM_OP" \
				--keys "$NUM_KEY" \
				--insert 0.5 \
				--search 0.3 \
				--elimination 0.2 \
				--repetitions $REPETITIONS \
				--outfile "${COUNT}.csv" \
				"$REGEX"
				COUNT=$((COUNT + 1))
		done
	done
done

mv -v ./*.csv output/
