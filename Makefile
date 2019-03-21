### Target Attributes
UNAME=$(shell uname)

### Flags
CC_OPT=\
	-Werror -Wextra -Wall -Wfatal-errors -pedantic\
 	-Wno-deprecated-register
CXX_OPT=\
	--std=c++14 -Werror -Wextra -Wall -Wfatal-errors -pedantic\
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

### Test targets
TEST_TARGET=bin/regression
BMARK_TARGET=bin/benchmark

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
	src/verilog/transform/de_alias.o\
	src/verilog/transform/dead_code_eliminate.o

### Header files
HDR=\
	src/base/bits/bits.h\
	src/base/container/vector.h\
	src/base/log/log.h\
	src/base/serial/serializable.h\
	src/base/stream/bufstream.h\
	src/base/stream/fdstream.h\
	src/base/stream/incstream.h\
	src/base/stream/indstream.h\
	src/base/stream/sockstream.h\
	src/base/stream/substream.h\
	src/base/system/system.h\
	src/base/thread/asynchronous.h\
	src/base/thread/thread_pool.h\
	src/base/token/tokenize.h\
	src/base/undo/undo_map.h\
	src/base/undo/undo_set.h\
	src/base/undo/undo_val.h\
	src/base/undo/undo_vector.h\
	src/base/undo/undoable.h\
	\
	src/runtime/data_plane.h\
	src/runtime/ids.h\
	src/runtime/isolate.h\
	src/runtime/module.h\
	src/runtime/runtime.h\
	\
	src/target/compiler.h\
	src/target/core.h\
	src/target/core_compiler.h\
	src/target/engine.h\
	src/target/input.h\
	src/target/interface.h\
	src/target/interface_compiler.h\
	src/target/state.h\
	src/target/common/remote_runtime.h\
	src/target/common/rpc.h\
	src/target/core/de10/de10_compiler.h\
	src/target/core/de10/de10_gpio.h\
	src/target/core/de10/de10_led.h\
	src/target/core/de10/de10_logic.h\
	src/target/core/de10/de10_pad.h\
	src/target/core/de10/io.h\
	src/target/core/de10/module_boxer.h\
	src/target/core/de10/program_boxer.h\
	src/target/core/de10/quartus_server.h\
	src/target/core/proxy/proxy_compiler.h\
	src/target/core/proxy/proxy_core.h\
	src/target/core/stub/stub_core.h\
	src/target/core/sw/monitor.h\
	src/target/core/sw/sw_clock.h\
	src/target/core/sw/sw_compiler.h\
	src/target/core/sw/sw_fifo.h\
	src/target/core/sw/sw_led.h\
	src/target/core/sw/sw_logic.h\
	src/target/core/sw/sw_memory.h\
	src/target/core/sw/sw_pad.h\
	src/target/core/sw/sw_reset.h\
	\
	src/ui/controller.h\
	src/ui/view.h\
	src/ui/combinator/many_view.h\
	src/ui/combinator/maybe_view.h\
	src/ui/log/log_view.h\
	src/ui/stream/stream_controller.h\
	src/ui/term/term_controller.h\
	src/ui/term/term_view.h\
	src/ui/web/web_ui.h\
	\
	src/verilog/analyze/constant.h\
	src/verilog/analyze/evaluate.h\
	src/verilog/analyze/indices.h\
	src/verilog/analyze/module_info.h\
	src/verilog/analyze/navigate.h\
	src/verilog/analyze/printf.h\
	src/verilog/analyze/read_set.h\
	src/verilog/analyze/resolve.h\
	src/verilog/ast/ast.h\
	src/verilog/ast/ast_fwd.h\
	src/verilog/ast/types/always_construct.h\
	src/verilog/ast/types/arg_assign.h\
	src/verilog/ast/types/assign_statement.h\
	src/verilog/ast/types/attr_spec.h\
	src/verilog/ast/types/attributes.h\
	src/verilog/ast/types/binary_expression.h\
	src/verilog/ast/types/block_statement.h\
	src/verilog/ast/types/blocking_assign.h\
	src/verilog/ast/types/case_generate_construct.h\
	src/verilog/ast/types/case_generate_item.h\
	src/verilog/ast/types/case_item.h\
	src/verilog/ast/types/case_statement.h\
	src/verilog/ast/types/concatenation.h\
	src/verilog/ast/types/conditional_expression.h\
	src/verilog/ast/types/conditional_generate_construct.h\
	src/verilog/ast/types/conditional_statement.h\
	src/verilog/ast/types/construct.h\
	src/verilog/ast/types/continuous_assign.h\
	src/verilog/ast/types/declaration.h\
	src/verilog/ast/types/delay_control.h\
	src/verilog/ast/types/display_statement.h\
	src/verilog/ast/types/error_statement.h\
	src/verilog/ast/types/event.h\
	src/verilog/ast/types/event_control.h\
	src/verilog/ast/types/expression.h\
	src/verilog/ast/types/finish_statement.h\
	src/verilog/ast/types/for_statement.h\
	src/verilog/ast/types/forever_statement.h\
	src/verilog/ast/types/get_statement.h\
	src/verilog/ast/types/generate_block.h\
	src/verilog/ast/types/generate_construct.h\
	src/verilog/ast/types/generate_region.h\
	src/verilog/ast/types/genvar_declaration.h\
	src/verilog/ast/types/id.h\
	src/verilog/ast/types/identifier.h\
	src/verilog/ast/types/if_generate_clause.h\
	src/verilog/ast/types/if_generate_construct.h\
	src/verilog/ast/types/info_statement.h\
	src/verilog/ast/types/initial_construct.h\
	src/verilog/ast/types/instantiation.h\
	src/verilog/ast/types/integer_declaration.h\
	src/verilog/ast/types/localparam_declaration.h\
	src/verilog/ast/types/loop_generate_construct.h\
	src/verilog/ast/types/loop_statement.h\
	src/verilog/ast/types/macro.h\
	src/verilog/ast/types/module_declaration.h\
	src/verilog/ast/types/module_instantiation.h\
	src/verilog/ast/types/module_item.h\
	src/verilog/ast/types/multiple_concatenation.h\
	src/verilog/ast/types/net_declaration.h\
	src/verilog/ast/types/node.h\
	src/verilog/ast/types/nonblocking_assign.h\
	src/verilog/ast/types/number.h\
	src/verilog/ast/types/par_block.h\
	src/verilog/ast/types/parameter_declaration.h\
	src/verilog/ast/types/put_statement.h\
	src/verilog/ast/types/port_declaration.h\
	src/verilog/ast/types/primary.h\
	src/verilog/ast/types/range_expression.h\
	src/verilog/ast/types/reg_declaration.h\
	src/verilog/ast/types/repeat_statement.h\
	src/verilog/ast/types/restart_statement.h\
	src/verilog/ast/types/retarget_statement.h\
	src/verilog/ast/types/save_statement.h\
	src/verilog/ast/types/scope.h\
	src/verilog/ast/types/seq_block.h\
	src/verilog/ast/types/statement.h\
	src/verilog/ast/types/string.h\
	src/verilog/ast/types/system_task_enable_statement.h\
	src/verilog/ast/types/timing_control.h\
	src/verilog/ast/types/timing_control_statement.h\
	src/verilog/ast/types/unary_expression.h\
	src/verilog/ast/types/variable_assign.h\
	src/verilog/ast/types/wait_statement.h\
	src/verilog/ast/types/warning_statement.h\
	src/verilog/ast/types/while_statement.h\
	src/verilog/ast/types/write_statement.h\
	\
	src/verilog/ast/visitors/builder.h\
	src/verilog/ast/visitors/editor.h\
	src/verilog/ast/visitors/rewriter.h\
	src/verilog/ast/visitors/visitor.h\
	\
	src/verilog/parse/lexer.h\
	src/verilog/parse/parser.h\
	\
	src/verilog/print/color.h\
	src/verilog/print/printer.h\
	src/verilog/print/debug/debug_printer.h\
	src/verilog/print/html/html_printer.h\
	src/verilog/print/term/term_printer.h\
	src/verilog/print/text/text_printer.h\
	\
	src/verilog/program/elaborate.h\
	src/verilog/program/inline.h\
	src/verilog/program/program.h\
	src/verilog/program/type_check.h\
	\
	src/verilog/transform/constant_prop.h\
	src/verilog/transform/dead_code_eliminate.h\
	src/verilog/transform/de_alias.h

### Test binaries
TEST_OBJ=\
	test/harness.o\
	test/regression/parse.o\
	test/regression/type_check.o\
	test/regression/simple.o\
	test/regression/array.o\
	test/regression/bitcoin.o\
	test/regression/mips32.o\
	test/regression/regex.o\
	test/regression/nw.o\
	test/regression/jit.o\
	test/regression/remote.o

### Benchmark binaries
BMARK_OBJ=\
	test/harness.o\
	test/benchmark/benchmark.o

### Tool binaries
BIN=\
	bin/cascade\
	bin/quartus_server\
	bin/sw_fpga

### Top-level commands
all: ${BIN}
test: ${TEST_TARGET}
	${TEST_TARGET}
benchmark: ${BMARK_TARGET}
	${BMARK_TARGET}
benchmark_de10: ${BMARK_TARGET}
	${BMARK_TARGET} --march de10_jit
clean:
	${MAKE} -C src/target/core/de10/fpga clean
	${RM} -rf ${GTEST_BUILD_DIR} ${TEST_TARGET} ${BMARK_TARGET} ${TEST_OBJ} 
	${RM} -rf ${FLEX_SRC} src/verilog/parse/*.hh src/verilog/parse/*.tab.* src/verilog/parse/*.output
	${RM} -rf ${OBJ} 
	${RM} -rf ${BIN}
	${RM} -rf bin/*.dSYM

### Build rules
bin/%: tools/%.cc ${FLEX_SRC} ${OBJ} ${HDR}
	ccache ${CXX} ${CXXFLAGS} ${CXX_OPT} ${PERF} ${INC} $< -o $@ ${OBJ} ${LIB}
${FLEX_SRC}: src/verilog/parse/verilog.ll ${BISON_SRC} ${HDR}
	cd src/verilog/parse && flex verilog.ll	
${BISON_SRC}: src/verilog/parse/verilog.yy ${HDR}
	cd src/verilog/parse && bison -d -v verilog.yy
%.o: %.c ${FLEX_SRC} ${BISON_SRC} ${HDR}
	ccache ${CC} ${CFLAGS} ${CC_OPT} ${PERF} ${GTEST_INC} ${INC} -c $< -o $@
%.o: %.cc ${FLEX_SRC} ${BISON_SRC} ${HDR}
	ccache ${CXX} ${CXXFLAGS} ${CXX_OPT} ${PERF} ${GTEST_INC} ${INC} -c $< -o $@
${GTEST_LIB}: 
	mkdir -p ${GTEST_BUILD_DIR}
	cd ${GTEST_BUILD_DIR} && CFLAGS=${CFLAGS} CXXFLAGS=${CXXFLAGS} cmake .. && make
${TEST_TARGET}: ${FLEX_SRC} ${OBJ} ${HDR} ${TEST_OBJ} ${GTEST_LIB} ${GTEST_MAIN}
	ccache ${CXX} ${CXXFLAGS} ${CXX_OPT} ${PERF} -o $@ ${TEST_OBJ} ${OBJ} ${GTEST_LIB} ${GTEST_MAIN} ${LIB} 
${BMARK_TARGET}: ${FLEX_SRC} ${OBJ} ${HDR} ${BMARK_OBJ} ${GTEST_LIB} ${GTEST_MAIN}
	ccache ${CXX} ${CXXFLAGS} ${CXX_OPT} ${PERF} -o $@ ${BMARK_OBJ} ${OBJ} ${GTEST_LIB} ${GTEST_MAIN} ${LIB} 

### Misc rules for targets that we don't control the source for
ext/mongoose/mongoose.o: ext/mongoose/mongoose.c
	ccache ${CC} ${CFLAGS} ${CC_OPT} ${PERF} -Wno-array-bounds -Wno-sign-compare -Wno-unused-parameter -Wno-format -Wno-format-pedantic ${GTEST_INC} ${INC} -c $< -o $@
src/verilog/parse/lex.yy.o: src/verilog/parse/lex.yy.cc 
	ccache ${CXX} ${CXXFLAGS} ${CXX_OPT} -march=native -Wno-null-conversion -fno-stack-protector -O3 -DNDEBUG -Wno-sign-compare ${GTEST_INC} ${INC} -c $< -o $@
src/verilog/parse/verilog.tab.o: src/verilog/parse/verilog.tab.cc
	ccache ${CXX} ${CXXFLAGS} ${CXX_OPT} -march=native -fno-stack-protector -O3 -DNDEBUG  ${GTEST_INC} ${INC} -c $< -o $@
