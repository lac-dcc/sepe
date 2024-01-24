#!/bin/sh

set -e

if [ "$(basename "$(pwd)")" = "scripts" ]; then
	cd ..
fi

if [ ! -d output ]; then
	mkdir output
fi

make

PHYSICAL_CORES=19
SPAWNED_PROCESSES=0
SPAWNED_PIDS=""
NUM_KEYS="500000 1000000"
NUM_OPS="1000000 50000000"
REGEXES="$(sed  -n 's/^\[\(.*\)\]/\1/p' Regexes.toml)"
REPETITIONS=3
DISTRIBUTIONS="
0.7 0.2 0.1
0.6 0.2 0.2
0.4 0.3 0.3
"
DISTRIBUTIONS_COUNT="$(echo DISTRIBUTIONS | wc -w)"

for REGEX in $REGEXES; do
	COUNT=0
	for NUM_OP in $NUM_OPS; do
		for NUM_KEY in $NUM_KEYS; do
			for ARG in $(seq 1 3 "$DISTRIBUTIONS_COUNT"); do
				if [ $SPAWNED_PROCESSES -ge "$PHYSICAL_CORES" ]; then
					# shellcheck disable=SC2086 # Intended splitting of PIDS
					wait $SPAWNED_PIDS
					SPAWNED_PIDS=""
					SPAWNED_PROCESSES=0
				fi

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
					"$REGEX" &

				SPAWNED_PIDS="$SPAWNED_PIDS $!"
				COUNT=$((COUNT + 1))
				SPAWNED_PROCESSES=$((SPAWNED_PROCESSES + 1))
			done
		done
	done
done

if [ -n "$SPAWNED_PIDS" ]; then
	# shellcheck disable=SC2086 # Intended splitting of PIDS
	wait $SPAWNED_PIDS
fi

mv -v ./*.csv output/
zip -9 -o "$(date '+%Y-%m-%d_%Hh-%Mm-%Ss')".zip -r output/*
