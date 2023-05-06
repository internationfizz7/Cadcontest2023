
if [ "$1" == "" ]; then
	echo "Provide benchmark Number as parameter!"
	exit
fi

if [ "$2" == "" ]; then
	echo "Provide bool for GSRC/IBM Flag!"
	echo "0 : for GSRC , 1 : for IBM"
	exit
fi



GSRCorIBM=$2 # 0 for GSRC , 1 for IBM
BenchPath=""
Layer=2
seed=0
OutputFile=./out
if [ $GSRCorIBM -eq 0 ]
    then
	#echo " - GSRC bench"
	benchName="n$1"
	BenchPath=bench/GSRC_SOFT/$benchName/$benchName
	Mode=1
	OutputFile=$OutputFile/$benchName

    else
    	#echo " - IBM bench"
	benchName=ibm$1
	BenchPath=bench/hb+/$benchName # The benchmark path
	Mode=2
	OutputFile=$OutputFile/$benchName

fi

time ./LA -format $Mode -bench $BenchPath -nparts $Layer -rseed $seed  -output 4 -OutputFile $OutputFile

