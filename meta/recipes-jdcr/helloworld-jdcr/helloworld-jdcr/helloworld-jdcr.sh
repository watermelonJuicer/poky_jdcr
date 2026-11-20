#!/bin/sh
# Write "HelloWorld N" to the given file every 5 seconds, N being an incrementing counter.

if [ $# -ne 1 ]; then
    echo "Usage: $0 <output-file>" >&2
    exit 1
fi

OUTFILE="$1"
N=1
while true; do
    echo "HelloWorld $N" >> "$OUTFILE"
    N=$((N + 1))
    sleep 5
done
