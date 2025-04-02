#!/usr/bin/env bash

export TCBSTART=./bin/minitcb_start 
export FADCEXE=./bin/nkfadc500_run
export JBNUEXE=./bin/jbnu_daq_run.exe
export RUNTIME=${1:-0}
export JBNUSID=${2:-3}
export NEVT=${3:-0}
export RUNNO=`cat RUNNUMBER`

echo "///////////////////"
echo "RUNNUMBER: ${RUNNO}"
echo "RUNTIME: ${RUNTIME} sec"
echo "///////////////////"
echo ""

$TCBSTART #192.168.0.2
$FADCEXE $RUNNO $NEVT &
$JBNUEXE $JBNUSID $RUNNO $NEVT &

if [ $RUNTIME -ne 0 ]; then
	sleep $RUNTIME
	source ./STOP.sh
fi

if [ $NEVT -ne 0 ]; then
	source ./STOP.sh
fi
