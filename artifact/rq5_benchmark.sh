#!/bin/sh

set -e

if [ "$(basename "$(pwd)")" = "scripts" ] || [ "$(basename "$(pwd)")" = "artifact"  ]; then
	cd ..
fi

if [ ! -d output ]; then
	mkdir output
fi

make -j"$(nproc)"
make -j"$(nproc)" benchmark

NUM_OPS=10000
NUM_KEYS="500 2000 10000"
REGEXES="$(sed  -n 's/^\[\(.*\)\]/\1/p' Regexes.toml)"
REPETITIONS=10
DISTRIBUTIONS="
0.7 0.2 0.1
0.6 0.2 0.2
0.4 0.3 0.3
"
DISTRIBUTIONS_COUNT="$(echo "$DISTRIBUTIONS" | wc -w)"

HISTOGRAM_DISTRIBUTION="normal uniform incremental"

for HD in $HISTOGRAM_DISTRIBUTION; do
    for REGEX in $REGEXES; do
        COUNT=0
        for NUM_OP in $NUM_OPS; do
            for NUM_KEY in $NUM_KEYS; do
                for ARG in $(seq 1 3 "$DISTRIBUTIONS_COUNT"); do

                    INSERTION="$(  echo "$DISTRIBUTIONS" | tr '\n' ' ' | awk "{print \$$((ARG + 0))}")"
                    SEARCH="$(     echo "$DISTRIBUTIONS" | tr '\n' ' ' | awk "{print \$$((ARG + 1))}")"
                    ELIMINATION="$(echo "$DISTRIBUTIONS" | tr '\n' ' ' | awk "{print \$$((ARG + 2))}")"

                    ./bin/sepe-runner \
                        --distribution "$HD" \
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
    zip -9 -o "RQ5-${HD}.zip" -r output/*
done

mkdir -p artifact/output-rq5
mv -- RQ5*.zip artifact/output-rq5
if [ ! -d output-rq3 ]; then
    ./artifact/rq3_benchmark.sh
fi
cd artifact/output-rq5
mkdir -p results

for DIST in $HISTOGRAM_DISTRIBUTION; 
do 
    unzip RQ5-"$DIST".zip -d rq5-data
    ../../scripts/global_keyuser_interpreter.py -p rq5-data/output/*.csv
    mv -v results/global_geomean.csv results/global_geomean_"$DIST".csv
done

echo "Done! Results are in artifact/output-rq5/results/"
