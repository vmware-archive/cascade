all: qsys quartus program

qsys:
	~/intelFPGA_lite/18.1/quartus/sopc_builder/bin/qsys-generate soc_system.qsys --synthesis=VERILOG

quartus:
	~/intelFPGA_lite/18.1/quartus/bin/quartus_map DE10_NANO_SoC_GHRD.qpf
	~/intelFPGA_lite/18.1/quartus/bin/quartus_fit DE10_NANO_SoC_GHRD.qpf
	~/intelFPGA_lite/18.1/quartus/bin/quartus_asm DE10_NANO_SoC_GHRD.qpf
	~/intelFPGA_lite/18.1/quartus/bin/quartus_cpf -c sof2rbf.cof

program:
	~/intelFPGA_lite/17.1/quartus/bin/quartus_pgm -c "DE-SoC [1-2]" --mode JTAG -o "P;output_files/DE10_NANO_SoC_GHRD.sof@2"

clean:
	rm -f soc_system.sopcinfo
	rm -rf db incremental_db greybox_tmp hps_isw_handoff output_files soc_system 
	rm -rf .qsys_edit
	rm -f *~ *.bak *.qws c5_pin_model_dump.txt
	cp ip/program_logic.template.v ip/program_logic.v
