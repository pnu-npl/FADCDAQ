#!/usr/bin/env bash

#Feb. 27, SID mapping: FADCs for 1 and 2; JBNU_DAQ for 3
export TCBINIT=./bin/minitcb_setup
#export JBNUINIT=./bin/jbnu_daq_set.exe
#export JBNUSID=${1:-3}
export RUNNO=`cat RUNNUMBER`

$TCBINIT -r $RUNNO -i 192.168.0.2 > log_${RUNNO}.txt
#$JBNUINIT $JBNUSID >> log_${RUNNO}.txt
cat log_${RUNNO}.txt
