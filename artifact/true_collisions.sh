#!/bin/sh

set -e

if [ ! -d output-rq3 ]; then
	echo "directory output-rq3 does not exist. Please run the \`rq3_benchmark.sh\` script"
	exit 1
fi

 tail -n 10 -- output-rq3/normal/output/*0.py |\
	 sed 's/\(UrlComplex\|Url\|SSN\|CPF\|INTS\|Hash\|IPV4\|IPV6\|Mac\)//g' |\
	 awk '/Function/ {sum[$3] += $5} END { for (hash in sum) print hash, sum[hash]; }' |\
	 sort
