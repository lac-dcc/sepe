#!/bin/sh

set -e

if [ $# -lt 1 ]; then
	echo "usage: $0 <path/to/csv/file>"
fi

column -o' ' -t -s, "$1"
