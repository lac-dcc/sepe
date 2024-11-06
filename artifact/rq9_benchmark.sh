#!/bin/bash

set -e

if [ ! -d output-rq1-rq2 ]; then
    ./rq1_rq2_benchmark.sh
fi

cd artifact/output-rq1-rq2
../../scripts/global_keyuser_interpreter.py -p performance/*.csv -rq9

echo "Done! Results are in artifact/output-rq1-rq2/results/"

