#!/bin/bash

#rm data

numRun=999

echo "n100_3_tier" >> data
for(( i=1; i<=$numRun; i+=1 ))
do
	./LA -format 1 -bench bench/GSRC_HARD/n100/n100 -nparts 3 -rseed $i -addName $i 
done

echo "n200_3_tier" >> data
for(( i=1; i<=$numRun; i+=1 ))
do
	./LA -format 1 -bench bench/GSRC_HARD/n200/n200 -nparts 3 -rseed $i -addName $i
done

echo "n300_3_tier" >> data
for(( i=1; i<=$numRun; i+=1 ))
do
	./LA -format 1 -bench bench/GSRC_HARD/n300/n300 -nparts 3 -rseed $i -addName $i
done

echo "n100_4_tier" >> data
for(( i=1; i<=$numRun; i+=1 ))
do
	./LA -format 1 -bench bench/GSRC_HARD/n100/n100 -nparts 4 -rseed $i -addName $i
done

echo "n200_4_tier" >> data
for(( i=1; i<=$numRun; i+=1 ))
do
	./LA -format 1 -bench bench/GSRC_HARD/n200/n200 -nparts 4 -rseed $i -addName $i
done

echo "n300_4_tier" >> data
for(( i=1; i<=$numRun; i+=1 ))
do
	./LA -format 1 -bench bench/GSRC_HARD/n300/n300 -nparts 4 -rseed $i -addName $i
done
