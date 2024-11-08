#!/bin/bash

set -e

if [ "$(basename "$(pwd)")" = "scripts" ] || [ "$(basename "$(pwd)")" = "artifact" ]; then
	cd ..
fi

mkdir -p artifact/output_rq8/

make -j"$(nproc)"
make -j"$(nproc)" bin/sepe-runner
make -B -j"$(nproc)" keyuser RQ8=true

KEY_SIZES="16 64 256 1024 4096 16384"
REPETITIONS=10
for KEY_SIZE in $KEY_SIZES; do
	# most arguments do not matter. We are just passing whatever for keyuser to work
	./bin/keyuser \
		-i 70 \
		-s 20 \
		-e 10 \
		-n 10000 \
		-r $REPETITIONS \
		--hashes AbseilHash CityHash FNVHash Pext"${KEY_SIZE}"INTS STDHashSrc \
		--hash-performance < artifact/rq6_input/INTS"$KEY_SIZE".dat \
		| awk "{ if (NR == 1) { print \$0 \",size\" } else { print \$0 \",$KEY_SIZE\" } }" \
		> artifact/output_rq8/"$KEY_SIZE".csv
done
rm -rf bin/keyuser output/*
cd artifact/output_rq8

# TODO: SCRIPT TO PLOT RQ8
