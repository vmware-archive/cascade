### Target Attributes
UNAME=$(shell uname)

### Constants: g++
CC=ccache gcc
CXX=ccache g++ --std=c++14
CC_OPT=\
	-Werror -Wextra -Wall -Wfatal-errors -pedantic\
 	-Wno-deprecated-register
CXX_OPT=\
	-Werror -Wextra -Wall -Wfatal-errors -pedantic\
 	-Wno-overloaded-virtual -Wno-deprecated-register
PERF=\
	-march=native -fno-exceptions -fno-stack-protector \
	-O3 -DNDEBUG
INC=-I. -I./ext/cl
LIB=-lncurses -lpthread

### Constants: gtest
GTEST_ROOT_DIR=ext/googletest/googletest
GTEST_BUILD_DIR=${GTEST_ROOT_DIR}/build
GTEST_INC_DIR=${GTEST_ROOT_DIR}/include
GTEST_MAIN=${GTEST_BUILD_DIR}/libgtest_main.a
GTEST_INC=-I${GTEST_INC_DIR}
GTEST_LIB=${GTEST_BUILD_DIR}/libgtest.a
GTEST_TARGET=bin/gtest

### Constants: auto-generated files
FLEX_SRC=src/verilog/parse/lex.yy.cc
BISON_SRC=src/verilog/parse/verilog.tab.cc

### Source binaries
OBJ=\
	ext/mongoose/mongoose.o\
	\
	src/runtime/data_plane.o\
	src/runtime/isolate.o\
	src/runtime/module.o\
	src/runtime/runtime.o\
	\
	src/target/common/remote_runtime.o\
	src/target/compiler.o\
	src/target/core/de10/de10_compiler.o\
	src/target/core/de10/de10_logic.o\
	src/target/core/de10/module_boxer.o\
	src/target/core/de10/program_boxer.o\
	src/target/core/de10/quartus_server.o\
	src/target/core/proxy/proxy_compiler.o\
	src/target/core/sw/sw_compiler.o\
	src/target/core/sw/sw_logic.o\
	src/target/core_compiler.o\
	src/target/input.o\
	src/target/interface_compiler.o\
	src/target/state.o\
	\
	src/ui/log/log_view.o\
	src/ui/stream/stream_controller.o\
	src/ui/term/term_controller.o\
	src/ui/term/term_view.o\
	src/ui/web/web_ui.o\
	\
	src/verilog/analyze/constant.o\
	src/verilog/analyze/evaluate.o\
	src/verilog/analyze/indices.o\
	src/verilog/analyze/module_info.o\
	src/verilog/analyze/navigate.o\
	src/verilog/analyze/printf.o\
	src/verilog/analyze/read_set.o\
	src/verilog/analyze/resolve.o\
	\
	src/verilog/ast/visitors/builder.o\
	src/verilog/ast/visitors/editor.o\
	src/verilog/ast/visitors/rewriter.o\
	src/verilog/ast/visitors/visitor.o\
	\
	src/verilog/parse/lex.yy.o\
	src/verilog/parse/parser.o\
	src/verilog/parse/verilog.tab.o\
	\
	src/verilog/print/printer.o\
	src/verilog/print/html/html_printer.o\
	src/verilog/print/term/term_printer.o\
	src/verilog/print/text/text_printer.o\
	\
	src/verilog/program/elaborate.o\
	src/verilog/program/inline.o\
	src/verilog/program/program.o\
	src/verilog/program/type_check.o\
	\
	src/verilog/transform/constant_prop.o\
	src/verilog/transform/de_alias.o

### Test binaries
TEST_OBJ=\
	test/harness.o\
	test/parse.o\
	test/type_check.o\
	test/simple.o\
	test/bitcoin.o\
	test/mips.o\
	test/regex.o\
	test/remote.o\
	test/jit.o

### Tool binaries
BIN=\
	bin/cascade\
	bin/quartus_server\
	bin/remote_runtime\
	bin/sw_fpga

### Top-level commands
all: ${BIN}
test: ${GTEST_TARGET}
	${MAKE} -C data/test/mips32/asm
	${MAKE} -C data/test/regex/codegen
	${MAKE} -C data/test/regex/data
	${GTEST_TARGET}
clean:
	${MAKE} -C src/target/core/de10/fpga clean
	${MAKE} -C data/test/mips32/asm clean
	${MAKE} -C data/test/regex/codegen clean
	${MAKE} -C data/test/regex/data clean
	${RM} -rf data/test/mips32/sc/*.mem
	${RM} -rf ${GTEST_BUILD_DIR} ${GTEST_TARGET} ${TEST_OBJ} 
	${RM} -rf ${FLEX_SRC} src/verilog/parse/*.hh src/verilog/parse/*.tab.* src/verilog/parse/*.output
	${RM} -rf ${OBJ} 
	${RM} -rf ${BIN}
	${RM} -rf bin/*.dSYM

### Build rules
submodule:
	git submodule init
	git submodule update
bin/%: tools/%.cc ${FLEX_SRC} ${OBJ}
	${CXX} ${CXXFLAGS} ${CXX_OPT} ${PERF} ${INC} $< -o $@ ${OBJ} ${LIB}
${FLEX_SRC}: src/verilog/parse/verilog.ll ${BISON_SRC}
	cd src/verilog/parse && flex verilog.ll	
${BISON_SRC}: src/verilog/parse/verilog.yy
	cd src/verilog/parse && bison -d -v verilog.yy
%.o: %.c ${FLEX_SRC} ${BISON_SRC}
	${CC} ${CFLAGS} ${CC_OPT} ${PERF} ${GTEST_INC} ${INC} -c $< -o $@
%.o: %.cc ${FLEX_SRC} ${BISON_SRC} 
	${CXX} ${CXXFLAGS} ${CXX_OPT} ${PERF} ${GTEST_INC} ${INC} -c $< -o $@
${GTEST_LIB}: submodule
	mkdir -p ${GTEST_BUILD_DIR}
	cd ${GTEST_BUILD_DIR} && CFLAGS=${CFLAGS} CXXFLAGS=${CXXFLAGS} cmake .. && make
${GTEST_TARGET}: ${FLEX_SRC} ${OBJ} ${TEST_OBJ} ${GTEST_LIB} ${GTEST_MAIN}
	${CXX} ${CXXFLAGS} ${CXX_OPT} ${PERF} -o $@ ${TEST_OBJ} ${OBJ} ${GTEST_LIB} ${GTEST_MAIN} ${LIB} 

### Misc rules for targets that we don't control the source for
ext/mongoose/mongoose.o: ext/mongoose/mongoose.c
	${CC} ${CFLAGS} ${CC_OPT} ${PERF} -Wno-sign-compare -Wno-unused-parameter -Wno-format -Wno-format-pedantic ${GTEST_INC} ${INC} -c $< -o $@
src/verilog/parse/lex.yy.o: src/verilog/parse/lex.yy.cc 
	${CXX} ${CXXFLAGS} ${CXX_OPT} -march=native -fno-stack-protector -O3 -DNDEBUG -Wno-sign-compare ${GTEST_INC} ${INC} -c $< -o $@
src/verilog/parse/verilog.tab.o: src/verilog/parse/verilog.tab.cc
	${CXX} ${CXXFLAGS} ${CXX_OPT} -march=native -fno-stack-protector -O3 -DNDEBUG  ${GTEST_INC} ${INC} -c $< -o $@
