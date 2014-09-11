lex -o ../src/lex.cpp lex.l
yacc --defines=../src/parse.h -o ../src/parse.cpp parse.y
