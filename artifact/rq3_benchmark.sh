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

rm -f -- *.zip

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
                        --histogram \
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
    zip -9 -o "histogram-${HD}.zip" -r output/*
done

DISTRIBS="uniform normal incremental"

mkdir -p artifact/output-rq3
mv -- *.zip artifact/output-rq3
cd artifact/output-rq3/

for DIST in $DISTRIBS; 
do 
    unzip histogram-"$DIST".zip -d "$DIST"
    cd "$DIST"/output/
    for f in *.csv; do mv "$f" "$(echo "$f" | cut -d. -f1)".py ; done
    cd ../../
    mkdir -p results
    ../../scripts/global_keyuser_interpreter.py -d "$DIST"/output/*.py
    cp results/URL_FIXED_PATTERN2_chitest.csv "$DIST"_result.csv
done

echo "Done, see results in artifact/output-rq3/results/"
