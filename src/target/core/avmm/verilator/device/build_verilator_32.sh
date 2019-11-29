#!/bin/sh

# Delete any previous compilations
rm -rf obj_dir >/dev/null

# Invoke verilator: fake_main.cpp is just here to guarantee that verilator produces all of the output we expect it to (namely obj_dir/verilated.o)
/usr/local/bin/verilator -Wno-lint -Wno-fatal -cc -O3 --x-assign fast --x-initial fast --noassert --clk clk program_logic.v --exe fake_main.cpp

# Invoke verilator's Makefile: We don't care about the binary this produces, all we're interested in are the object files (obj_dir/Vprogram_logic_ALL.a, obj_dir/verilated.o)
cd obj_dir
g++ -I.  -MMD -I/usr/local/share/verilator/include -I/usr/local/share/verilator/include/vltstd -DVL_PRINTF=printf -DVM_COVERAGE=0 -DVM_SC=0 -DVM_TRACE=0 -faligned-new -fbracket-depth=4096 -Qunused-arguments -Wno-parentheses-equality -Wno-sign-compare -Wno-uninitialized -Wno-unused-parameter -Wno-unused-variable -Wno-shadow  -O3 -fno-stack-protector -DNDEBUG -flto -DVL_INLINE_OPT=inline -c -o fake_main.o ../fake_main.cpp 
g++ -I.  -MMD -I/usr/local/share/verilator/include -I/usr/local/share/verilator/include/vltstd -DVL_PRINTF=printf -DVM_COVERAGE=0 -DVM_SC=0 -DVM_TRACE=0 -faligned-new -fbracket-depth=4096 -Qunused-arguments -Wno-parentheses-equality -Wno-sign-compare -Wno-uninitialized -Wno-unused-parameter -Wno-unused-variable -Wno-shadow  -O3 -fno-stack-protector -DNDEBUG -flto -DVL_INLINE_OPT=inline -c -o verilated.o /usr/local/share/verilator/include/verilated.cpp 
/usr/bin/perl /usr/local/share/verilator/bin/verilator_includer -DVL_INCLUDE_OPT=include Vprogram_logic.cpp > Vprogram_logic__ALLcls.cpp 
g++ -I.  -MMD -I/usr/local/share/verilator/include -I/usr/local/share/verilator/include/vltstd -DVL_PRINTF=printf -DVM_COVERAGE=0 -DVM_SC=0 -DVM_TRACE=0 -faligned-new -fbracket-depth=4096 -Qunused-arguments -Wno-parentheses-equality -Wno-sign-compare -Wno-uninitialized -Wno-unused-parameter -Wno-unused-variable -Wno-shadow  -O3 -fno-stack-protector -DNDEBUG -flto -DVL_INLINE_OPT=inline -c -o Vprogram_logic__ALLcls.o Vprogram_logic__ALLcls.cpp 
/usr/bin/perl /usr/local/share/verilator/bin/verilator_includer -DVL_INCLUDE_OPT=include Vprogram_logic__Syms.cpp > Vprogram_logic__ALLsup.cpp 
g++ -I.  -MMD -I/usr/local/share/verilator/include -I/usr/local/share/verilator/include/vltstd -DVL_PRINTF=printf -DVM_COVERAGE=0 -DVM_SC=0 -DVM_TRACE=0 -faligned-new -fbracket-depth=4096 -Qunused-arguments -Wno-parentheses-equality -Wno-sign-compare -Wno-uninitialized -Wno-unused-parameter -Wno-unused-variable -Wno-shadow  -O3 -fno-stack-protector -DNDEBUG -flto -DVL_INLINE_OPT=inline -c -o Vprogram_logic__ALLsup.o Vprogram_logic__ALLsup.cpp 
ar r Vprogram_logic__ALL.a Vprogram_logic__ALLcls.o Vprogram_logic__ALLsup.o 
ranlib Vprogram_logic__ALL.a 
cd -

# Compile our harness file, which wraps invocations of verilator in extern "C" functions. 
g++ --std=c++14 -fno-stack-protector -DNDEBUG -flto -I/usr/local/opt/verilator/share/verilator/include/ -Iobj_dir -c harness_32.cpp -o obj_dir/harness.o

# Wrap everything up in a dll
g++ -shared -flto -o obj_dir/libverilator.so obj_dir/harness.o obj_dir/Vprogram_logic__ALL.a obj_dir/verilated.o
