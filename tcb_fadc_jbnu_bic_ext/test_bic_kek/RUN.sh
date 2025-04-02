#!/usr/bin/env bash

if [ "$#" -ne 2 ]; then
	echo "usage : ./RUN.sh <nevts> <prescale>"
	exit 1
fi

#export FADCEXE=./bin/nkfadc500_run
export BICEXE=./bin/BIC_DAQ_run.exe
export RUNNO=`cat RUNNUMBER`
export RUNEVENT=$1
export PRESCALE=$2

RUNNO=$((RUNNO+1))


echo "==================="
echo "RUNNUMBER: ${RUNNO}"
echo "RUNEVENTS: ${RUNEVENT} evts"
echo "PRESCALE: ${PRESCALE}"
echo "==================="
echo ""

sed -i "7s/.*/$PRESCALE/" ./setup/setup_41.txt
sed -i "7s/.*/$PRESCALE/" ./setup/setup_42.txt
sed -i "8s/.*/$PRESCALE/" ./setup/setup_31.txt

echo $RUNNO > RUNNUMBER
#$FADCEXE $RUNNO &
#sleep 1
./INIT.sh
$BICEXE $RUNNO $RUNEVENT &

#if [ $RUNTIME -ne 0 ]; then
#	sleep $RUNTIME
#	source ./STOP.sh
#fi
