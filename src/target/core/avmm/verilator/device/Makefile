lib32: 
	rm -rf obj_dir \
		# Delete any previous compilations
	/usr/local/bin/verilator -Wno-lint -Wno-fatal -cc -O3 --x-assign fast --x-initial fast --noassert --clk clk program_logic.v --exe fake_main.cpp  \
		# Invoke verilator: fake_main.cpp is just here to guarantee that verilator produces all of the output we expect it to (namely obj_dir/verilated.o)
	make -C obj_dir OPT="-O3 -fno-stack-protector -DNDEBUG -flto -DVL_INLINE_OPT=inline" -f Vprogram_logic.mk \
	  # Invoke verilator's Makefile: We don't care about the binary this produces, all we're interested in are the object files (obj_dir/Vprogram_logic_ALL.a, obj_dir/verilated.o)
	g++ --std=c++14 -fno-stack-protector -DNDEBUG -flto -I/usr/local/opt/verilator/share/verilator/include/ -Iobj_dir -c harness_32.cpp -o obj_dir/harness.o \
	  # Compile our harness file, which wraps invocations of verilator in extern "C" functions. 
	g++ -shared -flto -o obj_dir/libverilator.so obj_dir/harness.o obj_dir/Vprogram_logic__ALL.a obj_dir/verilated.o \
		# Wrap everything up in a dll

lib64: 
	rm -rf obj_dir \
		# Delete any previous compilations
	/usr/local/bin/verilator -Wno-lint -Wno-fatal -cc -O3 --x-assign fast --x-initial fast --noassert --clk clk program_logic.v --exe fake_main.cpp  \
		# Invoke verilator: fake_main.cpp is just here to guarantee that verilator produces all of the output we expect it to (namely obj_dir/verilated.o)
	make -C obj_dir OPT="-O3 -fno-stack-protector -DNDEBUG -flto -DVL_INLINE_OPT=inline" -f Vprogram_logic.mk \
	  # Invoke verilator's Makefile: We don't care about the binary this produces, all we're interested in are the object files (obj_dir/Vprogram_logic_ALL.a, obj_dir/verilated.o)
	g++ --std=c++14 -fno-stack-protector -DNDEBUG -flto -I/usr/local/opt/verilator/share/verilator/include/ -Iobj_dir -c harness_64.cpp -o obj_dir/harness.o \
	  # Compile our harness file, which wraps invocations of verilator in extern "C" functions. 
	g++ -shared -flto -o obj_dir/libverilator.so obj_dir/harness.o obj_dir/Vprogram_logic__ALL.a obj_dir/verilated.o \
		# Wrap everything up in a dll
