
## This shell is to generate power
# If case is GSRC, amp = 10. case is ibm , amp = 100
./clean.sh
CaseName=n100
CaseBlockNum=100
InputPath=../../
Amp=10
dytime=1

./Power -filename $CaseName -casenum $CaseBlockNum -datapath $InputPath -amp $Amp -dy $dytime
