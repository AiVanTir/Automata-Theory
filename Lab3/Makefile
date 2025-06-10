# Makefile для проекта «robot_interpreter» (Flex + Bison)

CC      = gcc
LEX     = flex
YACC    = bison
CFLAGS  = -O2 -Wall -I.
LDFLAGS = -lfl

OBJS = lex.yy.o \
       robot_lang.tab.o \
       ast.o \
       interp.o \
       maze.o \
       main.o

all: robot_interpreter

robot_interpreter: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

robot_lang.tab.c robot_lang.tab.h: robot_lang.y ast.h
	$(YACC) -d -v -Wcounterexamples robot_lang.y

lex.yy.c: robot_lang.l robot_lang.tab.h
	$(LEX) robot_lang.l

%.o: %.c
	$(CC) $(CFLAGS) -c $<

main.o: main.c robot_lang.tab.h

clean:
	rm -f *.o lex.yy.c robot_lang.tab.c robot_lang.tab.h robot_lang.output robot_interpreter

.PHONY: all clean

