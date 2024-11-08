#!/bin/bash

set -e

if [ "$(basename "$(pwd)")" = "scripts" ] || [ "$(basename "$(pwd)")" = "artifact" ]; then
	cd ..
fi

if [ ! -d output ]; then
	mkdir output
fi

pwd

make -j"$(nproc)"
make -j"$(nproc)" benchmark

KEY_SIZES="16 64 256 1024 4096 16384"

rm -rf rq6.csv output/* artifact/output_rq6/
echo "REGEX,Exec Time,Problem Size" >> rq6.csv
for SIZE in $KEY_SIZES; do
	for ((i = 0 ; i < 10 ; i++)); do
		# disable shellcheck for unquoted $(), since we are doing this deliberately
		# shellcheck disable=SC2046
		VAL=$(./bin/keysynth _rq6 $(./bin/keybuilder < artifact/rq6_input/INTS"$SIZE".dat))
		echo "${VAL}" >> rq6.csv
	done
done
rm -rf output/*

cd artifact/
mkdir -p output_rq6/
mv -v ../rq6.csv output_rq6/

mkdir -p results/
../scripts/keyuser_interpreter.py -rq6 output_rq6/rq6.csv
mv -v results/rq6.pdf output_rq6/
echo "Results are in output_rq6/rq6.pdf"
