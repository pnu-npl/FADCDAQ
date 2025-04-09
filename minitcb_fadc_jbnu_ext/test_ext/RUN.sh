#!/usr/bin/env bash

export TCBSTART=./bin/minitcb_start 
export FADCEXE=./bin/nkfadc500_run
export JBNUEXE=./bin/jbnu_daq_run.exe
export RUNNO=`cat RUNNUMBER`
export RUNTIME=${1:-0}

echo "///////////////////"
echo "RUNNUMBER: ${RUNNO}"
echo "RUNTIME: ${RUNTIME} sec"
echo "///////////////////"
echo ""

$TCBSTART #192.168.0.2
$FADCEXE $RUNNO &; sleep 1
$JBNUEXE 21 $RUNNO 0 &

if [ $RUNTIME -ne 0 ]; then
	sleep $RUNTIME
	source ./STOP.sh
fi
