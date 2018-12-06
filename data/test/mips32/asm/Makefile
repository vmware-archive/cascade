all:
	bison -d mips32.yy
	flex mips32.ll
	g++ ${CXXFLAGS} -std=c++14 -c lex.yy.cc
	g++ ${CXXFLAGS} -std=c++14 -c mips32.tab.cc
	g++ ${CXXFLAGS} -std=c++14 main.cc *.o -o asm

clean:
	rm -f lex.yy.cc location.hh position.hh stack.hh *.tab.* *.o asm
