#!/bin/sh

set -e

if [ "$(basename "$(pwd)")" = "scripts" ] || [ "$(basename "$(pwd)")" = "artifact"  ]; then
        cd ..
fi

if [ ! -d output ]; then
        mkdir output
fi

rm -f output/*.csv 

make -j"$(nproc)"
make -j"$(nproc)" benchmark
DISTRIBUTIONS="normal"
NUM_OPS=10000
NUM_KEYS="500 2000 10000"
REGEXES="$(sed  -n 's/^\[\(.*\)\]/\1/p' Regexes.toml)"
REPETITIONS=10
PERCENTAGES="
0.7 0.2 0.1
0.6 0.2 0.2
0.4 0.3 0.3
"
PERCENTAGES_COUNT="$(echo "$PERCENTAGES" | wc -w)"

for REGEX in $REGEXES; do
        COUNT=0
		for DISTRIBUTION in $DISTRIBUTIONS; do
			for NUM_OP in $NUM_OPS; do
					for NUM_KEY in $NUM_KEYS; do
							for ARG in $(seq 1 3 "$PERCENTAGES_COUNT"); do

									INSERTION="$(  echo "$PERCENTAGES" | tr '\n' ' ' | awk "{print \$$((ARG + 0))}")"
									SEARCH="$(     echo "$PERCENTAGES" | tr '\n' ' ' | awk "{print \$$((ARG + 1))}")"
									ELIMINATION="$(echo "$PERCENTAGES" | tr '\n' ' ' | awk "{print \$$((ARG + 2))}")"

									./bin/sepe-runner \
											--verbose \
											--distribution "$DISTRIBUTION" \
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
done

mv -v ./*.csv output/
zip -9 -o RQ1_RQ2.zip -r output/*

for REGEX in $REGEXES; do
        COUNT=0
		for DISTRIBUTION in $DISTRIBUTIONS; do
			for NUM_OP in $NUM_OPS; do
					for NUM_KEY in $NUM_KEYS; do
							for ARG in $(seq 1 3 "$PERCENTAGES_COUNT"); do

									INSERTION="$(  echo "$PERCENTAGES" | tr '\n' ' ' | awk "{print \$$((ARG + 0))}")"
									SEARCH="$(     echo "$PERCENTAGES" | tr '\n' ' ' | awk "{print \$$((ARG + 1))}")"
									ELIMINATION="$(echo "$PERCENTAGES" | tr '\n' ' ' | awk "{print \$$((ARG + 2))}")"

									./bin/sepe-runner \
											--verbose \
											--hash-performance \
											--distribution "$DISTRIBUTION" \
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
done

mv -v ./*.csv output/
zip -9 -o RQ1_RQ2_hash-performance.zip -r output/*

mkdir -p artifact/output-rq1-rq2
mv -- RQ1_RQ2*.zip artifact/output-rq1-rq2
cd artifact/output-rq1-rq2/

mkdir -p results/

unzip RQ1_RQ2_hash-performance.zip -d hash-performance
../../scripts/global_keyuser_interpreter.py -p hash-performance/output/*.csv -hp > results/hash-performance.txt

unzip RQ1_RQ2.zip -d performance
../../scripts/global_keyuser_interpreter.py -p performance/*.csv

echo "Done! Results are in artifact/output-rq1-rq2/results/"
