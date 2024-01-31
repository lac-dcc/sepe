#!/bin/sh

set -e

if [ "$(basename "$(pwd)")" = "scripts" ]; then
	cd ..
fi

if [ ! -d output ]; then
	mkdir output
fi

make

NUM_OPS=10000
NUM_KEYS="500 2000 10000"
REGEXES="$(sed  -n 's/^\[\(.*\)\]/\1/p' Regexes.toml)"
REPETITIONS=100
DISTRIBUTIONS="
0.7 0.2 0.1
0.6 0.2 0.2
0.4 0.3 0.3
"
DISTRIBUTIONS_COUNT="$(echo "$DISTRIBUTIONS" | wc -w)"

for REGEX in $REGEXES; do
	COUNT=0
	for NUM_OP in $NUM_OPS; do
		for NUM_KEY in $NUM_KEYS; do
			for ARG in $(seq 1 3 "$DISTRIBUTIONS_COUNT"); do

				INSERTION="$(  echo "$DISTRIBUTIONS" | tr '\n' ' ' | awk "{print \$$((ARG + 0))}")"
				SEARCH="$(     echo "$DISTRIBUTIONS" | tr '\n' ' ' | awk "{print \$$((ARG + 1))}")"
				ELIMINATION="$(echo "$DISTRIBUTIONS" | tr '\n' ' ' | awk "{print \$$((ARG + 2))}")"

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
		done
	done
done

mv -v ./*.csv output/
zip -9 -o "$(date '+%Y-%m-%d_%Hh-%Mm-%Ss')".zip -r output/*