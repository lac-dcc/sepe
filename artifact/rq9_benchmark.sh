#!/bin/bash

set -e

if [ ! -d output-rq1-rq2 ]; then
    ./rq1_rq2_benchmark.sh
fi

cd output-rq1-rq2/
../../scripts/global_keyuser_interpreter.py -p performance/output/*.csv -rq9
mkdir -p ../output-rq9
mv -v results/containers.pdf ../output-rq9/

echo "Done! Results are in artifact/output-rq9/containers.pdf"

