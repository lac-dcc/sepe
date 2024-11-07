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
REPETITIONS=10

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

#TODO: SCRIPT TO PLOT RQ6

mkdir -p output_rq8/
cd ..
for KEY_SIZE in $KEY_SIZES; do
	# most arguments do not matter. We are just passing whatever for keyuser to work
	./bin/keyuser \
		-i 70 \
		-s 20 \
		-e 10 \
		-n 10000 \
		-r $REPETITIONS \
		--hashes AbseilHash CityHash FNVHash PextINTS STDHashSrc \
		--hash-performance < artifact/rq6_input/INTS"$KEY_SIZE".dat \
		> artifact/output_rq8/"$KEY_SIZE".csv
done

rm -rf output/*
cd artifact/output_rq8

# TODO: SCRIPT TO PLOT RQ8
