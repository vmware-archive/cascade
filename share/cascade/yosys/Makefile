all: root.flash a.out

%.json: %.v
	yosys \
		-p "synth_ecp5 -json $@" \
		-E .$(basename $@).d \
		$<
%.config: %.json
	nextpnr-ecp5 \
		--json $< \
		--textcfg $@ \
		--lpf ulx3s_v20.lpf \
		--85k \
		--package CABGA381
%.bit: %.config
	ecppack --idcode 0x41113043 $< $@
%.flash: %.bit
	ujprog -b 3000000 $<

a.out: main.cc
	g++ --std=c++17 main.cc 

clean:
	$(RM) *.json *.config *.bit a.out
