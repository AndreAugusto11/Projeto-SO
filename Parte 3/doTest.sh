#!/bin/bash

#cd CircuitRouter-SeqSolver
#make
#cd ../CircuitRouter-ParSolver
#make

 
cd CircuitRouter-SeqSolver
./CircuitRouter-SeqSolver $2

fileName=$2
fileName+=".res"
resultsName1=$2
resultsName1+=".speedups.csv"
resultsName="results/"
resultsName+=$resultsName1

read seqTime < <(grep "Elapsed time" $fileName |cut -d\= -f2 |cut -d\s -f1)
cd ..
echo Threads,exec_time,speedup > $resultsName

speedup=$(echo "scale=6; ${seqTime}/${seqTime}" | bc)
echo 1S, $seqTime, $speedup >> $resultsName

cd CircuitRouter-ParSolver
for ((i=1; i<=$1; i++))
do
	./CircuitRouter-ParSolver -t $i $2
	read parTime < <(grep "Elapsed time" $fileName |cut -d\= -f2 |cut -d\s -f1)
	speedup=$(echo "scale=6; ${seqTime}/${parTime}" | bc)
	cd ..
	echo $i, $parTime, $speedup >> $resultsName
	cd CircuitRouter-ParSolver
done