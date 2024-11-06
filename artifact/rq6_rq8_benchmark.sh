#!/bin/bash

set -e

if [ "$(basename "$(pwd)")" = "scripts" ] || [ "$(basename "$(pwd)")" = "artifact"  ]; then
	cd ..
fi

if [ ! -d output ]; then
        mkdir output
fi

pwd

make -j"$(nproc)"
make -j"$(nproc)" benchmark

KEY_SIZES="16 64 256 1024 4096 16384"
DISTRIBUTIONS="normal"
NUM_OPS=10000
NUM_KEYS="1000"
REGEXES="$(sed  -n 's/^\[\(.*\)\]/\1/p' Regexes.toml)"
REPETITIONS=10
PERCENTAGES="
0.7 0.2 0.1
0.6 0.2 0.2
0.4 0.3 0.3
"
PERCENTAGES_COUNT="$(echo "$PERCENTAGES" | wc -w)"

rm -rf output/*
rm -f rq6.csv
rm -rf artifact/output_rq6/
echo "REGEX,Exec Time,Problem Size" >> rq6.csv
for SIZE in $KEY_SIZES; do
	for ((i = 0 ; i < 10 ; i++)); do	
		VAL=$(./bin/keysynth _rq6 $(cat artifact/rq6_input/INTS$SIZE.dat | ./bin/keybuilder))
		echo "${VAL}" >> rq6.csv
	done
done
rm -rf output/*

cd artifact/
mkdir -p output_rq6/
mv -v ../rq6.csv output_rq6/

#TODO: SCRIPT TO PLOT

for REGEX in $REGEXES; do
        # COUNT=0
		for DISTRIBUTION in $DISTRIBUTIONS; do
		for NUM_OP in $NUM_OPS; do
					for KEY_SIZE in $KEY_SIZES; do
							for ARG in $(seq 1 3 "$PERCENTAGES_COUNT"); do

									INSERTION="$(  echo "$PERCENTAGES" | tr '\n' ' ' | awk "{print \$$((ARG + 0))}")"
									SEARCH="$(     echo "$PERCENTAGES" | tr '\n' ' ' | awk "{print \$$((ARG + 1))}")"
									ELIMINATION="$(echo "$PERCENTAGES" | tr '\n' ' ' | awk "{print \$$((ARG + 2))}")"

									./bin/sepe-runner \
											--verbose \
											--hash-performance \
											--distribution "$DISTRIBUTION" \
											--operations "$NUM_OP" \
											--keys "$NUM_KEYS" \
											--insert "$INSERTION" \
											--search "$SEARCH" \
											--elimination "$ELIMINATION" \
											--repetitions $REPETITIONS \
											--outfile "RQ8_${DISTRIBUTION}_hash_functions.csv" \
											"$REGEX"

							done
					done
			done
		done
done

mv -v ./*.csv output/
zip -9 -o RQ8.zip -r output/*
rm -rf output/*

cd artifact/
mkdir -p output_rq8/
mv -v ../RQ8.zip output_rq8/
cd output_rq8/
unzip RQ8.zip

# TODO: SCRIPT TO PLOT

