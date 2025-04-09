#!/usr/bin/env bash

#Apr. 9, MID mapping: FADCs for 1 and 2; JBNU_DAQ for 21
export TCBINIT=./bin/minitcb_setup
export RUNNO=`cat RUNNUMBER`

$TCBINIT -r $RUNNO -i 192.168.0.2 > log_${RUNNO}.txt
cat log_${RUNNO}.txt
