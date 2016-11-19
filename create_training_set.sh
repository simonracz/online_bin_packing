#!/bin/sh
die () {
    echo >&2 "$@"
    exit 1
}

[ "$#" -eq 1 ] || die "Usage:\n        $0 output_file"

#base
for i in `seq 0 0.1 0.9`;
do
	for j in `seq 1 20`
	do
		./generate -l 12 -r $i | ./annotate --auto -f $1
	done
done

#core
for i in `seq 0 0.1 0.4`;
do
	for j in `seq 1 50`
	do
		./generate -l 12 -r $i | ./annotate --auto -f $1
	done
done