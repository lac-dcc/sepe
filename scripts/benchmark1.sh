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
NUM_OPS="1000000"
REGEXES="$(sed  -n 's/^\[\(.*\)\]/\1/p' Regexes.toml)"
REPETITIONS="10"
DISTRIBUTIONS="0.7 0.2 0.1
0.6 0.2 0.2
0.5 0.3 0.2
0.4 0.3 0.3"

for REGEX in $REGEXES; do
	COUNT=0
	for NUM_KEY in $NUM_KEYS; do
		for NUM_OP in $NUM_OPS; do
			echo "$DISTRIBUTIONS" | while IFS= read -r DISTRIBUTION; do
				INSERTION="$(echo "$DISTRIBUTION" | awk '{print $1}')"
				SEARCH="$(echo "$DISTRIBUTION" | awk '{print $2}')"
				ELIMINATION="$(echo "$DISTRIBUTION" | awk '{print $3}')"

				./bin/bench-runner \
					--verbose \
					--operations "$NUM_OP" \
					--keys "$NUM_KEY" \
					--insert "$INSERTION" \
					--search "$SEARCH" \
					--elimination "$ELIMINATION" \
					--repetitions $REPETITIONS \
					--outfile "${COUNT}.csv" \
					"$REGEX"
				COUNT=$((COUNT + 1))
			done
			COUNT=$((COUNT + 4))
		done
	done
done

mv -v ./*.csv output/
zip -o "$(date '+%Y-%m-%d_%Hh-%Mm-%Ss')".zip -r output/*
