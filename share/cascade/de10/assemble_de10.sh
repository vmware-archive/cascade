#!/bin/sh

#1 = path/to/intel/tools

$1/bin/quartus_asm DE10_NANO_SoC_GHRD.qpf
$1/bin/quartus_cpf -c sof2rbf.cof
