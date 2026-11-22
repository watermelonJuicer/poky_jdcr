#!/bin/sh
# Write "HelloWorld N" to the given file every 5 seconds, N being an incrementing counter.

# MESSAGE and OUTFILE are environment-values. 
while true; do
    echo "$MESSAGE $N" >> "$OUTFILE"
    N=$((N + 1))
    sleep $INTERVAL
done
