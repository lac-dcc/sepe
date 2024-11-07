#!/bin/sh

set -e

if [ "$(basename "$(pwd)")" = "scripts" ] || [ "$(basename "$(pwd)")" = "artifact"  ]; then
	cd ..
fi

if [ ! -d output ]; then
	mkdir output
fi

git checkout upper-shift-experiment

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

mkdir -p temp-files

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
	zip -9 -o "rq7-${HD}.zip" -r temp-files/*
done

rm -f temp-files/*

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
										--verbose \
										--distribution "normal" \
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
zip -9 -o rq7-performance.zip -r temp-files/*

git checkout artifact

mkdir -p output-rq7
mv ../*.zip output-rq7

cd output-rq7
mkdir -p results

unzip rq7-performance.zip
mv temp-files performance

for dist in incremental normal uniform; do
	unzip rq7-$dist.zip
	mv temp-files $dist
	cd $dist
    for f in *.csv; do mv "$f" "$(echo "$f" | cut -d. -f1)".py ; done
	cd ..
done
unzip rq7-incremental.zip
mv temp-files incremental
unzip rq7-normal.zip
mv temp-files normal
unzip rq7-uniform.zip
mv temp-files uniform

../../scripts/global_keyuser_interpreter.py -p performance/*.csv

# TODO: how the hell do we run this??? 
../../scripts/global_keyuser_interpreter.py -d incremental/*.py
