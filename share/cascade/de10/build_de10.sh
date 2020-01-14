#!/bin/sh

#1 = path/to/intel/tools

$1/sopc_builder/bin/qsys-generate soc_system.qsys --synthesis=VERILOG &&
$1/bin/quartus_map DE10_NANO_SoC_GHRD.qpf &&
$1/bin/quartus_fit DE10_NANO_SoC_GHRD.qpf
