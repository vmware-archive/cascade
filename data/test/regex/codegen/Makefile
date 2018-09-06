all:
	bison -d regex.yy
	flex regex.ll
	g++ -std=c++14 -c lex.yy.cc
	g++ -std=c++14 -c regex.tab.cc
	g++ -std=c++14 -c nfa.cc
	g++ -std=c++14 main.cc *.o -o codegen

clean:
	rm -f lex.yy.cc location.hh position.hh stack.hh *.tab.* *.o *.v codegen 
