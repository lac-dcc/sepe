#!/bin/sh

set -e

if [ $# -lt 1 ]; then
	echo "usage: $0 <path/to/csv/file>"
fi

ALIGNED="$(column -o' ' -t -s, "$1")"
echo "$ALIGNED" | head -n 1
echo "$ALIGNED" | tail -n +2 | sort -k 8
