#!/usr/bin/env bash

#make clean
make

#echo ""
#echo COMPILE "Makefile_daq"
#echo "##########################################################"
#make -f Makefile_daq

echo ""
echo COMPILE "Makefile_run"
echo "##########################################################"
make -f Makefile_run

#echo ""
#echo COMPILE "Makefile_set"
#echo "##########################################################"
#make -f Makefile_set

mv *.o *.exe ./bin
