#!/bin/bash

for FILE1 in "$@"
do
	compressed=$FILE1".he"
	time ./huffc $FILE1 $compressed
	time ./huffd $compressed $FILE1".de"
	diff $FILE1 $FILE1".de"

	before=$(wc -c < "$FILE1")
	#echo $before

	after=$(wc -c < "$compressed")
	#echo $after
	rate=$after"/"$before
	rate="scale=2; $(echo -e "${rate}" | tr -d '[:space:]')"
	echo "$rate" | bc -l
	rm -f $compressed
	rm -f $FILE1".de"
done