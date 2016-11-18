#!/bin/sh

#base
for i in `seq 0 0.1 0.8`;
do
	for j in `seq 1 20`
	do
		./generate -l 10 -r $i | ./annotate --auto
	done
done

#core
for i in `seq 0 0.1 0.3`;
do
	for j in `seq 1 50`
	do
		./generate -l 10 -r $i | ./annotate --auto
	done
done