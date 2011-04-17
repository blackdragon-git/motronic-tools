all: parser

clean:
	rm parser.cpp parser.hpp parser tokens.cpp

parser.cpp: parser.y
	bison -d -o $@ $^
	
parser.hpp: parser.cpp

tokens.cpp: tokens.l parser.hpp
	lex -o $@ $^

parser: parser.cpp main.cpp tokens.cpp xdfGen.cpp util.cpp util.h node.h node.cpp XmlStream.hpp
	g++ -g -o $@ parser.cpp main.cpp tokens.cpp xdfGen.cpp util.cpp node.cpp
