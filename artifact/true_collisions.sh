#!/bin/sh

set -e

if [ ! -d output-rq3 ]; then
	echo "directory output-rq3 does not exist. Please run the \`rq3_benchmark.sh\` script"
	exit 1
fi

if [ $# -lt 1 ]; then
	echo "usage: $0 <normal|incremental|uniform>"
	exit 2
fi

case $1 in
	normal | incremental | uniform)
		tail -n 10 -- output-rq3/"$1"/output/*0.py |
			sed 's/\(UrlComplex\|Url\|SSN\|CPF\|INTS\|Hash\|IPV4\|IPV6\|Mac\)//g' |
			awk '/Function/ {sum[$3] += $5} END { for (hash in sum) print hash, sum[hash]; }' |
			sort
		;;
	*)
		echo "usage: $0 <normal|incremental|uniform>"
		exit 3
		;;
esac
