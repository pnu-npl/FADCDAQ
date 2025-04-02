#!/usr/bin/env bash

export FADCPROC=nkfadc500_run
export JBNUPROC=jbnu_daq_run.exe
export TCBSTOP=./bin/minitcb_stop
export FADCDECODE=./bin/nkfadc500_decode
export JBNUDECODE=./bin/jbnu_daq_decode
export RUNNO=`cat RUNNUMBER`

#: '
# Stop current run and minitcb
PIDFADC=`pidof $FADCPROC`
for i in $(echo $PIDFADC)
do
    kill -s SIGINT $i
	#echo "Close FADC..."
done
#'

#: '
PIDJBNU=`pidof $JBNUPROC`
for i in $(echo $PIDJBNU)
do
    kill -s SIGINT $i
	#echo "Close JBNU_DAQ..."
done
$TCBSTOP
echo "RUN: $RUNNO finished!"
#'

echo "Move dat files to ./data in 5 sec..."
sleep 5

# Decode dat file if it exists, ckim
#if compgen -G ./FADCData_${RUNNO}*.dat > /dev/null; then
if compgen -G ./*_${RUNNO}*.dat > /dev/null; then
	mkdir -p ./data
	mv ./log*.txt ./*.dat ./data
	$FADCDECODE $RUNNO
	$JBNUDECODE $RUNNO
fi

# Increase Run number by 1 for next run
RUNNO=`expr $RUNNO + 1`
echo "$RUNNO" > RUNNUMBER
