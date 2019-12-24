#!/bin/bash

# $1 = unique compilation name

cd $1

yosys -q -p "synth_ecp5 -json root32.json" root32.v
nextpnr-ecp5 --json root32.json --textcfg root32.config --lpf ulx3s_v20.lpf --85k --package CABGA381
ecppack --idcode 0x41113043 root32.config root32.bit

cd -
