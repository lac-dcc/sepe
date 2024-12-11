#!/bin/sh

set -e

if [ "$(basename "$(pwd)")" = "scripts" ] || [ "$(basename "$(pwd)")" = "artifact"  ]; then
	cd ..
fi

if [ ! -d output ]; then
	mkdir output
fi

# TODO: switch branch to upper-shifts

UPPER_SHIFTS="0 16 24 32 48"
NUM_OPS=10000
NUM_KEYS="10000"
REGEXES="$(sed  -n 's/^\[\(.*\)\]/\1/p' Regexes.toml)"
REPETITIONS=1
DISTRIBUTIONS="
0.7 0.2 0.1
0.6 0.2 0.2
0.4 0.3 0.3
"
DISTRIBUTIONS_COUNT="$(echo "$DISTRIBUTIONS" | wc -w)"

HISTOGRAM_DISTRIBUTION="normal uniform incremental"

for HD in $HISTOGRAM_DISTRIBUTION; do
	for SHIFT in $UPPER_SHIFTS; do
		make clean
		make -j"$(nproc)"
		make -j"$(nproc)" benchmark UPPER_SHIFT="$SHIFT"

		for REGEX in $REGEXES; do
			COUNT=0
			for NUM_OP in $NUM_OPS; do
				for NUM_KEY in $NUM_KEYS; do
					for ARG in $(seq 1 3 "$DISTRIBUTIONS_COUNT"); do

						INSERTION="$(  echo "$DISTRIBUTIONS" | tr '\n' ' ' | awk "{print \$$((ARG + 0))}")"
						SEARCH="$(     echo "$DISTRIBUTIONS" | tr '\n' ' ' | awk "{print \$$((ARG + 1))}")"
						ELIMINATION="$(echo "$DISTRIBUTIONS" | tr '\n' ' ' | awk "{print \$$((ARG + 2))}")"

						./bin/sepe-runner \
							--histogram \
							--distribution "$HD" \
							--verbose \
							--operations "$NUM_OP" \
							--keys "$NUM_KEY" \
							--insert "$INSERTION" \
							--search "$SEARCH" \
							--elimination "$ELIMINATION" \
							--repetitions $REPETITIONS \
							--outfile "_${COUNT}_SHIFT_${SHIFT}.csv" \
							"$REGEX"

						COUNT=$((COUNT + 1))
					done
				done
			done
		done
		mv -v ./*.csv temp-files/
	done
	zip -9 -o "histogram-${HD}.zip" -r temp-files/*
done



# TODO: switch back branch to artifact