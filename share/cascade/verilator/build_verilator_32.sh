#!/bin/sh

# $1 = unique compilation name
# $2 = cxx compiler path

# Check whether cxx compiler maps to clang or g++
$2 --version | grep clang 
if [ $? -eq 0 ] ; then
  VER_INSTALL=/usr/local/share/verilator/
  ARGS="-fbracket-depth=4096 -Qunused-arguments"
else 
  VER_INSTALL=/usr/share/verilator/
  ARGS="-ftemplate-depth=4096 -fconstexpr-depth=4096"
fi

# Invoke verilator: fake_main.cpp is just here to guarantee that verilator produces all of the output we expect it to (namely $1/verilated.o)
verilator -Mdir $1 --prefix Vprogram_logic -Wno-lint -Wno-fatal -cc -O3 --x-assign fast --x-initial fast --noassert --clk clk $1.v --exe fake_main.cpp

# Invoke verilator's Makefile: We don't care about the binary this produces, all we're interested in are the object files ($1/Vprogram_logic_ALL.a, $1/verilated.o)
cd $1
$2 -I.  -MMD -I$VER_INSTALL/include -I$VER_INSTALL/include/vltstd -DVL_PRINTF=printf -DVM_COVERAGE=0 -DVM_SC=0 -DVM_TRACE=0 -faligned-new $ARGS -Wno-parentheses-equality -Wno-sign-compare -Wno-uninitialized -Wno-unused-parameter -Wno-unused-variable -Wno-shadow  -O3 -fno-stack-protector -DNDEBUG -flto -DVL_INLINE_OPT=inline -c -o verilated.o $VER_INSTALL/include/verilated.cpp 
perl $VER_INSTALL/bin/verilator_includer -DVL_INCLUDE_OPT=include Vprogram_logic.cpp > Vprogram_logic__ALLcls.cpp 
$2 -I.  -MMD -I$VER_INSTALL/include -I$VER_INSTALL/include/vltstd -DVL_PRINTF=printf -DVM_COVERAGE=0 -DVM_SC=0 -DVM_TRACE=0 -faligned-new $ARGS -Wno-parentheses-equality -Wno-sign-compare -Wno-uninitialized -Wno-unused-parameter -Wno-unused-variable -Wno-shadow  -O3 -fno-stack-protector -DNDEBUG -flto -DVL_INLINE_OPT=inline -c -o Vprogram_logic__ALLcls.o Vprogram_logic__ALLcls.cpp 
perl $VER_INSTALL/bin/verilator_includer -DVL_INCLUDE_OPT=include Vprogram_logic__Syms.cpp > Vprogram_logic__ALLsup.cpp 
$2 -I.  -MMD -I$VER_INSTALL/include -I$VER_INSTALL/include/vltstd -DVL_PRINTF=printf -DVM_COVERAGE=0 -DVM_SC=0 -DVM_TRACE=0 -faligned-new $ARGS -Wno-parentheses-equality -Wno-sign-compare -Wno-uninitialized -Wno-unused-parameter -Wno-unused-variable -Wno-shadow  -O3 -fno-stack-protector -DNDEBUG -flto -DVL_INLINE_OPT=inline -c -o Vprogram_logic__ALLsup.o Vprogram_logic__ALLsup.cpp 
ar r Vprogram_logic__ALL.a Vprogram_logic__ALLcls.o Vprogram_logic__ALLsup.o 
ranlib Vprogram_logic__ALL.a 
cd -

# Compile our harness file, which wraps invocations of verilator in extern "C" functions. 
$2 --std=c++17 -fno-stack-protector -DNDEBUG -flto -I$VER_INSTALL/include/ -I$1 -c harness_32.cpp -o $1/harness.o

# Wrap everything up in a dll
$2 -fPIC -shared -flto -o $1/libverilator.so $1/harness.o $1/Vprogram_logic__ALL.a $1/verilated.o
